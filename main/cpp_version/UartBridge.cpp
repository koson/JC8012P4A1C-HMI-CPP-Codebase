#include "UartBridge.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "UartBridge";

static bool s_initialized = false;

// ── Init / Deinit ─────────────────────────────────────────────────────────────

void uart_bridge_init(void)
{
    if (s_initialized)
        return;

    const uart_config_t cfg = {
        .baud_rate = UART_BRIDGE_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = 0,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_BRIDGE_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_BRIDGE_PORT,
                                 UART_BRIDGE_TX_PIN,
                                 UART_BRIDGE_RX_PIN,
                                 UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_BRIDGE_PORT, 256, 256, 0, NULL, 0));

    s_initialized = true;
    ESP_LOGI(TAG, "UART%d init OK  TX=%d RX=%d %dbaud",
             UART_BRIDGE_PORT, UART_BRIDGE_TX_PIN, UART_BRIDGE_RX_PIN,
             UART_BRIDGE_BAUD);
}

void uart_bridge_deinit(void)
{
    if (!s_initialized)
        return;
    uart_driver_delete(UART_BRIDGE_PORT);
    s_initialized = false;
    ESP_LOGI(TAG, "UART%d deinit", UART_BRIDGE_PORT);
}

// ── Internal: SCPI transact ───────────────────────────────────────────────────

static uart_bridge_err_t scpi_transact(const char *cmd,
                                       char *resp, size_t resp_len)
{
    if (!s_initialized)
        return UB_ERR_NOT_INIT;

    uart_flush_input(UART_BRIDGE_PORT);
    uart_write_bytes(UART_BRIDGE_PORT, cmd, strlen(cmd));
    uart_write_bytes(UART_BRIDGE_PORT, "\n", 1);

    // Read byte-by-byte until '\n' or deadline
    size_t idx = 0;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(UART_BRIDGE_TIMEOUT_MS);

    while (xTaskGetTickCount() < deadline)
    {
        uint8_t b = 0;
        int n = uart_read_bytes(UART_BRIDGE_PORT, &b, 1, pdMS_TO_TICKS(100));
        if (n < 1)
            continue;
        if (b == '\n')
            break;
        if (idx < resp_len - 1)
            resp[idx++] = (char)b;
    }
    resp[idx] = '\0';
    if (idx > 0 && resp[idx - 1] == '\r')
        resp[--idx] = '\0';

    if (idx == 0)
    {
        ESP_LOGW(TAG, "cmd='%s' timeout", cmd);
        return UB_ERR_TIMEOUT;
    }

    ESP_LOGD(TAG, "cmd='%s' resp='%s'", cmd, resp);

    if (strncmp(resp, "ERROR:", 6) == 0)
    {
        ESP_LOGW(TAG, "H7 error: %s", resp);
        return UB_ERR_H7;
    }

    return UB_OK;
}

// ── Public SCPI commands ──────────────────────────────────────────────────────

uart_bridge_err_t uart_bridge_idn(char *buf, size_t len)
{
    return scpi_transact("*IDN?", buf, len);
}

uart_bridge_err_t uart_bridge_reset(void)
{
    char resp[32];
    return scpi_transact("*RST", resp, sizeof(resp));
}

uart_bridge_err_t uart_bridge_conf_output(uint8_t dip_pin)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "CONF:PIN:MODE %u,OUTPUT", dip_pin);
    char resp[32];
    return scpi_transact(cmd, resp, sizeof(resp));
}

uart_bridge_err_t uart_bridge_conf_input(uint8_t dip_pin)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "CONF:PIN:MODE %u,INPUT", dip_pin);
    char resp[32];
    return scpi_transact(cmd, resp, sizeof(resp));
}

uart_bridge_err_t uart_bridge_set_pin(uint8_t dip_pin, uint8_t val)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "DIG:OUT %u,%u", dip_pin, val & 1u);
    char resp[32];
    return scpi_transact(cmd, resp, sizeof(resp));
}

uart_bridge_err_t uart_bridge_read_pin(uint8_t ch, uint8_t *val)
{
    if (!val)
        return UB_ERR_NOT_INIT;
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "DIG:IN? %u", ch);
    char resp[32];
    uart_bridge_err_t err = scpi_transact(cmd, resp, sizeof(resp));
    if (err == UB_OK)
        *val = (uint8_t)atoi(resp);
    return err;
}

uart_bridge_err_t uart_bridge_pwr(uint8_t on)
{
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "PWR %u", on & 1u);
    char resp[32];
    return scpi_transact(cmd, resp, sizeof(resp));
}

uart_bridge_err_t uart_bridge_port_out(uint8_t mask)
{
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "PORT:OUT %u", mask);
    char resp[32];
    return scpi_transact(cmd, resp, sizeof(resp));
}

uart_bridge_err_t uart_bridge_port_in(uint8_t *mask)
{
    if (!mask)
        return UB_ERR_NOT_INIT;
    char resp[32];
    uart_bridge_err_t err = scpi_transact("PORT:IN?", resp, sizeof(resp));
    if (err == UB_OK)
        *mask = (uint8_t)atoi(resp);
    return err;
}
