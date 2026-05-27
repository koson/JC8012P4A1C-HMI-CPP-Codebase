#include "UartBridge.h"

#include "usb_cdc_host.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "UartBridge";

static bool s_initialized = false;
static bool s_host_installed = false;

// ── Init / Deinit ─────────────────────────────────────────────────────────────

void uart_bridge_init(void)
{
    // Intentionally a no-op: USB host is installed lazily on the first SCPI
    // command (in scpi_transact).  Installing at startup would allocate USB DMA
    // buffers from SPIRAM before LVGL allocates its framebuffers, corrupting
    // the display flush path (esp_cache_msync null-pointer crash).
}

void uart_bridge_deinit(void)
{
    s_initialized = false;
    // USB host daemon continues running; it will reconnect on next open attempt.
}

// ── Internal: ensure USB host + device open ───────────────────────────────────

static uart_bridge_err_t ensure_connected(void)
{
    // 1. Install USB host stack once (allocates DMA buffers — deferred to here)
    if (!s_host_installed)
    {
        esp_err_t err = usb_cdc_host_install();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "usb_cdc_host_install: %s", esp_err_to_name(err));
            return UB_ERR_NOT_INIT;
        }
        s_host_installed = true;
        // Give USB daemon task time to enumerate devices already on the bus.
        // In ESP32_LabBuddy the host starts at boot and H7 needs ~400 ms to
        // enumerate; mirror that here for the lazy-install path.
        vTaskDelay(pdMS_TO_TICKS(600));
    }

    // 2. If previously opened, check still alive; reset if H7 disconnected
    if (s_initialized)
    {
        if (usb_cdc_host_is_connected())
            return UB_OK;
        // H7 disconnected since last open — allow re-open
        s_initialized = false;
        ESP_LOGW(TAG, "H7 disconnected — will re-open");
    }

    // 3. Open the device — 1500 ms gives enough time for first enumeration.
    //    If H7 is already enumerated this returns in <50 ms.
    esp_err_t err = usb_cdc_host_open(USB_BRIDGE_VID, USB_BRIDGE_PID, 0, 1500);
    if (err != ESP_OK)
    {
        return UB_ERR_NOT_INIT;
    }
    s_initialized = true;
    ESP_LOGI(TAG, "USB CDC H7 connected  VID=%04X PID=%04X",
             USB_BRIDGE_VID, USB_BRIDGE_PID);
    return UB_OK;
}

// ── Internal: SCPI transact ───────────────────────────────────────────────────

static uart_bridge_err_t scpi_transact(const char *cmd,
                                       char *resp, size_t resp_len)
{
    uart_bridge_err_t ub = ensure_connected();
    if (ub != UB_OK)
    {
        ESP_LOGW(TAG, "cmd='%s' — H7 not connected", cmd);
        return ub;
    }

    esp_err_t err = usb_cdc_host_send(cmd);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "cmd='%s' send failed: %s", cmd, esp_err_to_name(err));
        return UB_ERR_TIMEOUT;
    }

    err = usb_cdc_host_recv_line(resp, (int)resp_len, UART_BRIDGE_TIMEOUT_MS);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "cmd='%s' recv timeout", cmd);
        return UB_ERR_TIMEOUT;
    }

    // Strip trailing CR if present (H7 sends "OK\r\n", usb_cdc_host strips \n)
    size_t rlen = strlen(resp);
    if (rlen > 0 && resp[rlen - 1] == '\r')
        resp[--rlen] = '\0';

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
