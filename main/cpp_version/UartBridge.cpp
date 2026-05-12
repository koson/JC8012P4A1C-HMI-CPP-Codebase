#include "UartBridge.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <string.h>

static const char *TAG = "UartBridge";

static bool s_initialized = false;

// ── Init / Deinit ─────────────────────────────────────────────────────────────

void uart_bridge_init(void)
{
    if (s_initialized) return;

    const uart_config_t cfg = {
        .baud_rate  = UART_BRIDGE_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags      = 0,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_BRIDGE_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_BRIDGE_PORT,
                                 UART_BRIDGE_TX_PIN,
                                 UART_BRIDGE_RX_PIN,
                                 UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    // 256-byte TX buffer, 256-byte RX buffer, no event queue
    ESP_ERROR_CHECK(uart_driver_install(UART_BRIDGE_PORT, 256, 256, 0, NULL, 0));

    s_initialized = true;
    ESP_LOGI(TAG, "UART%d init OK — TX=%d RX=%d baud=%d",
             UART_BRIDGE_PORT, UART_BRIDGE_TX_PIN, UART_BRIDGE_RX_PIN,
             UART_BRIDGE_BAUD);
}

void uart_bridge_deinit(void)
{
    if (!s_initialized) return;
    uart_driver_delete(UART_BRIDGE_PORT);
    s_initialized = false;
    ESP_LOGI(TAG, "UART%d deinit", UART_BRIDGE_PORT);
}

// ── Internal: send command and receive response ───────────────────────────────

/**
 * @brief Send a 4-byte command frame and wait for a 2-byte response.
 *
 * Frame layout:
 *   TX: [0xBB][cmd][param_hi][param_lo]
 *   RX: [0xCC][result]
 *
 * @param cmd       Command byte (UB_CMD_*)
 * @param param_hi  High byte of parameter (0 for most commands)
 * @param param_lo  Low byte of parameter
 * @param[out] result  Result byte from H7 (only valid when UB_OK returned)
 * @return UB_OK, UB_ERR_TIMEOUT, or UB_ERR_FRAMING
 */
static uart_bridge_err_t send_command(uint8_t cmd,
                                      uint8_t param_hi,
                                      uint8_t param_lo,
                                      uint8_t *result)
{
    if (!s_initialized) return UB_ERR_NOT_INIT;

    // Flush any stale RX data before sending
    uart_flush_input(UART_BRIDGE_PORT);

    // Send 4-byte command frame
    uint8_t tx_buf[4] = { UB_START_CMD, cmd, param_hi, param_lo };
    uart_write_bytes(UART_BRIDGE_PORT, (const char *)tx_buf, sizeof(tx_buf));

    // Wait for 2-byte response with timeout
    uint8_t rx_buf[2] = {};
    int rx_len = uart_read_bytes(UART_BRIDGE_PORT,
                                 rx_buf,
                                 sizeof(rx_buf),
                                 pdMS_TO_TICKS(UART_BRIDGE_TIMEOUT_MS));
    if (rx_len < 2) {
        ESP_LOGW(TAG, "cmd=0x%02X timeout (got %d bytes)", cmd, rx_len);
        return UB_ERR_TIMEOUT;
    }

    if (rx_buf[0] != UB_START_RSP) {
        ESP_LOGW(TAG, "cmd=0x%02X bad response header 0x%02X", cmd, rx_buf[0]);
        return UB_ERR_FRAMING;
    }

    *result = rx_buf[1];
    return UB_OK;
}

// ── Public commands ───────────────────────────────────────────────────────────

uart_bridge_err_t uart_bridge_set_output(uint8_t pin_mask)
{
    uint8_t result = 0;
    uart_bridge_err_t err = send_command(UB_CMD_SET_OUTPUT, 0, pin_mask, &result);
    if (err == UB_OK) {
        ESP_LOGD(TAG, "set_output mask=0x%02X → ack=0x%02X", pin_mask, result);
    }
    return err;
}

uart_bridge_err_t uart_bridge_read_input(uint8_t *pin_mask)
{
    if (!pin_mask) return UB_ERR_NOT_INIT;

    uint8_t result = 0;
    uart_bridge_err_t err = send_command(UB_CMD_READ_INPUT, 0, 0, &result);
    if (err == UB_OK) {
        *pin_mask = result;
        ESP_LOGD(TAG, "read_input → 0x%02X", result);
    }
    return err;
}
