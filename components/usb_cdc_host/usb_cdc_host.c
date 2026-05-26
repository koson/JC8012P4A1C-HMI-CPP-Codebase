/**
 * @file usb_cdc_host.c
 * @brief Reusable USB CDC-ACM Host — implementation
 *
 * Uses the IDF usb_host_cdc_acm class driver directly (no esp_modem layer).
 * RX is buffered line-by-line; a binary semaphore is given on each '\n'
 * so usb_cdc_host_recv_line() can block efficiently.
 *
 * Thread safety: usb_cdc_host_send() is serialised with a mutex.
 * usb_cdc_host_recv_line() is intended for one reader at a time.
 */

#include "usb_cdc_host.h"
#include "usb/usb_host.h"
#include "usb/cdc_acm_host.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "USB_CDC_HOST";

// ── Internal state ────────────────────────────────────────────────────────────

static cdc_acm_dev_hdl_t s_dev = NULL;
static volatile bool s_connected = false;

// TX serialisation
static SemaphoreHandle_t s_tx_mutex = NULL;

// RX line buffer + semaphore
// on_rx_data appends bytes; gives s_rx_sem when '\n' is seen.
// usb_cdc_host_recv_line() waits on s_rx_sem then copies s_rx_line.
static char s_rx_line[USB_CDC_HOST_LINE_MAX];
static size_t s_rx_len = 0;
static SemaphoreHandle_t s_rx_sem = NULL;

// ── CDC-ACM callbacks (called from cdc_acm_client_task — NOT ISR) ─────────────

static bool on_rx_data(const uint8_t *data, size_t len, void *arg)
{
    for (size_t i = 0; i < len; i++)
    {
        char c = (char)data[i];
        if (c == '\r')
            continue;
        if (c == '\n')
        {
            s_rx_line[s_rx_len] = '\0';
            s_rx_len = 0;
            xSemaphoreGive(s_rx_sem); // signal one line ready
        }
        else if (s_rx_len < USB_CDC_HOST_LINE_MAX - 1)
        {
            s_rx_line[s_rx_len++] = c;
        }
    }
    return true;
}

static void on_dev_event(const cdc_acm_host_dev_event_data_t *ev, void *ctx)
{
    if (ev->type == CDC_ACM_HOST_DEVICE_DISCONNECTED)
    {
        ESP_LOGW(TAG, "H7 disconnected");
        s_connected = false;
        s_dev = NULL;
        xSemaphoreGive(s_rx_sem); // unblock any waiting recv_line
    }
}

// ── USB Host daemon task (required by IDF USB Host lib) ───────────────────────

static void usb_daemon_task(void *arg)
{
    for (;;)
    {
        uint32_t flags = 0;
        usb_host_lib_handle_events(portMAX_DELAY, &flags);
        if (flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
            usb_host_device_free_all();
        if (flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
            break;
    }
    vTaskDelete(NULL);
}

// ── Public API ────────────────────────────────────────────────────────────────

esp_err_t usb_cdc_host_install(void)
{
    s_rx_sem = xSemaphoreCreateBinary();
    s_tx_mutex = xSemaphoreCreateMutex();
    if (!s_rx_sem || !s_tx_mutex)
        return ESP_ERR_NO_MEM;

    const usb_host_config_t host_cfg = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    esp_err_t ret = usb_host_install(&host_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "usb_host_install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    xTaskCreate(usb_daemon_task, "usb_daemon", 4096, NULL, 10, NULL);

    const cdc_acm_host_driver_config_t drv_cfg = {
        .driver_task_stack_size = 4096,
        .driver_task_priority = 10,
        .xCoreID = 0,
        .new_dev_cb = NULL,
    };
    ret = cdc_acm_host_install(&drv_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "cdc_acm_host_install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "USB Host + CDC-ACM driver ready");
    return ESP_OK;
}

esp_err_t usb_cdc_host_open(uint16_t vid, uint16_t pid, uint8_t iface,
                            uint32_t timeout_ms)
{
    s_rx_len = 0;
    xSemaphoreTake(s_rx_sem, 0); // drain any stale semaphore count

    const cdc_acm_host_device_config_t dev_cfg = {
        .connection_timeout_ms = timeout_ms,
        .out_buffer_size = 512,
        .in_buffer_size = 512,
        .event_cb = on_dev_event,
        .data_cb = on_rx_data,
        .user_arg = NULL,
    };
    esp_err_t ret = cdc_acm_host_open(vid, pid, iface, &dev_cfg, &s_dev);
    if (ret == ESP_OK)
    {
        s_connected = true;
        ESP_LOGI(TAG, "CDC-ACM open — VID=%04X PID=%04X iface=%u", vid, pid, iface);
    }
    else
    {
        ESP_LOGE(TAG, "cdc_acm_host_open failed (%s) — VID=%04X PID=%04X iface=%u",
                 esp_err_to_name(ret), vid, pid, iface);
    }
    return ret;
}

bool usb_cdc_host_is_connected(void)
{
    return s_connected;
}

esp_err_t usb_cdc_host_send(const char *cmd)
{
    if (!s_connected || !s_dev)
        return ESP_ERR_INVALID_STATE;

    char buf[USB_CDC_HOST_LINE_MAX + 4];
    int n = snprintf(buf, sizeof(buf), "%s\r\n", cmd);

    xSemaphoreTake(s_tx_mutex, portMAX_DELAY);
    esp_err_t ret = cdc_acm_host_data_tx_blocking(
        s_dev, (const uint8_t *)buf, (size_t)n, 2000);
    xSemaphoreGive(s_tx_mutex);

    if (ret == ESP_OK)
        ESP_LOGD(TAG, "TX → %s", cmd);
    else
        ESP_LOGE(TAG, "TX failed: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t usb_cdc_host_recv_line(char *out, size_t len, uint32_t timeout_ms)
{
    if (xSemaphoreTake(s_rx_sem, pdMS_TO_TICKS(timeout_ms)) == pdFALSE)
        return ESP_ERR_TIMEOUT;
    if (!s_connected)
        return ESP_ERR_INVALID_STATE;

    strncpy(out, s_rx_line, len - 1);
    out[len - 1] = '\0';
    ESP_LOGD(TAG, "RX ← %s", out);
    return ESP_OK;
}
