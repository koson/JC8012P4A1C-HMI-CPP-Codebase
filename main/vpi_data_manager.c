/**
 * @file vpi_data_manager.c
 * @brief VPI Data Manager Implementation
 */

#include "vpi_data_manager.h"
#include "ui_vpi_hmi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "VPI_DATA";

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static vpi_data_t vpi_data = {0};
static SemaphoreHandle_t data_mutex = NULL;
static TaskHandle_t monitor_task_handle = NULL;
static bool manager_running = false;
static vpi_document_handle_t vpi_document_handle = NULL; // VPIDocument handle for logging

// ============================================================================
// EXTERNAL UI UPDATE FUNCTIONS
// ============================================================================

extern void ui_vpi_hmi_update_electrical(uint8_t phase, float voltage, float current,
                                         float power, float reactive, float energy);
extern void ui_vpi_hmi_update_pressure(float percentage);
extern void ui_vpi_hmi_update_status(const char *status);

// ============================================================================
// UART CALLBACKS
// ============================================================================

/**
 * @brief PM2230 data callback
 */
static void on_pm2230_data(uint8_t node_id, const uart_pm2230_data_t *data)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        vpi_data.pm2230.valid = true;
        vpi_data.pm2230.last_update = esp_log_timestamp();
        vpi_data.pm2230.meter_id = data->meter_id;

        // Voltage
        vpi_data.pm2230.voltage_an = data->voltage_an;
        vpi_data.pm2230.voltage_bn = data->voltage_bn;
        vpi_data.pm2230.voltage_cn = data->voltage_cn;

        // Current
        vpi_data.pm2230.current_a = data->current_a;
        vpi_data.pm2230.current_b = data->current_b;
        vpi_data.pm2230.current_c = data->current_c;

        // Power
        vpi_data.pm2230.power_active = data->power_active;
        vpi_data.pm2230.power_reactive = data->power_reactive;
        vpi_data.pm2230.power_apparent = data->power_apparent;
        vpi_data.pm2230.power_factor = data->power_factor;

        // Energy
        vpi_data.pm2230.energy_total = data->energy_total;

        // Frequency
        vpi_data.pm2230.frequency = data->frequency;
        vpi_data.pm2230.error_flags = data->error_flags;

        // Update status
        vpi_data.status.pm2230_online = true;
        vpi_data.status.last_pm2230_rx = esp_log_timestamp();

        xSemaphoreGive(data_mutex);

        // Update VPIDocument if available
        if (vpi_document_handle != NULL)
        {
            pm2230_data_t pm2230_snapshot = {
                .voltage_a = data->voltage_an,
                .voltage_b = data->voltage_bn,
                .voltage_c = data->voltage_cn,
                .current_a = data->current_a,
                .current_b = data->current_b,
                .current_c = data->current_c,
                .power = (uint32_t)data->power_active};
            vpi_document_update_pm2230(vpi_document_handle, &pm2230_snapshot);
        }

        // Update UI outside mutex (avoid blocking LVGL)
        // Phase A (Red)
        ui_vpi_hmi_update_electrical(1, data->voltage_an, data->current_a,
                                     data->power_active / 3.0f, // Distribute total power among phases
                                     data->power_reactive / 3.0f,
                                     data->energy_total / 3.0f);

        // Phase B (Yellow)
        ui_vpi_hmi_update_electrical(2, data->voltage_bn, data->current_b,
                                     data->power_active / 3.0f,
                                     data->power_reactive / 3.0f,
                                     data->energy_total / 3.0f);

        // Phase C (Blue)
        ui_vpi_hmi_update_electrical(3, data->voltage_cn, data->current_c,
                                     data->power_active / 3.0f,
                                     data->power_reactive / 3.0f,
                                     data->energy_total / 3.0f);

        ESP_LOGI(TAG, "[PM2230] V=%.1f/%.1f/%.1f V, I=%.2f/%.2f/%.2f A, P=%.0f W",
                 data->voltage_an, data->voltage_bn, data->voltage_cn,
                 data->current_a, data->current_b, data->current_c,
                 data->power_active);
    }
}

/**
 * @brief 4-20mA sensor data callback
 */
static void on_4_20ma_data(uint8_t node_id, const uart_4_20ma_data_t *data)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        vpi_data.sensors.valid = true;
        vpi_data.sensors.last_update = esp_log_timestamp();
        vpi_data.sensors.adc_status = data->adc_status;

        // Process each channel
        for (int i = 0; i < data->num_channels && i < 4; i++)
        {
            const uart_4_20ma_channel_t *ch = &data->channels[i];

            if (ch->sensor_type == SENSOR_TYPE_PRESSURE)
            {
                // Store raw current value
                vpi_data.sensors.pressure_current_ma = ch->current_ma;

                // Convert 4-20mA to percentage (0-100%)
                // 4mA = 0%, 20mA = 100%
                float percentage = (ch->current_ma - 4.0f) / 16.0f * 100.0f;
                if (percentage < 0)
                    percentage = 0;
                if (percentage > 100)
                    percentage = 100;
                vpi_data.sensors.pressure_percent = percentage;

                // Store engineering values (will update when datasheet is available)
                vpi_data.sensors.pressure_bar = ch->sensor_value;
                vpi_data.sensors.pressure_psi = ch->sensor_value * 14.5038f; // bar to PSI
                vpi_data.sensors.pressure_quality = ch->quality;

                ESP_LOGI(TAG, "[DATA] Pressure: %.2f mA (%.0f%%), %.3f bar (%.1f PSI), Quality: %d%%",
                         ch->current_ma, percentage,
                         vpi_data.sensors.pressure_bar,
                         vpi_data.sensors.pressure_psi,
                         ch->quality);
                ESP_LOGD(TAG, "[CAL] Sensor: %s, Range: %s",
                         PRESSURE_SENSOR_DESCRIPTION, PRESSURE_SENSOR_RANGE_STR);
            }
            else if (ch->sensor_type == SENSOR_TYPE_FLOW)
            {
                // Flow sensor (L/min)
                vpi_data.sensors.flow_lpm = ch->sensor_value;
                vpi_data.sensors.flow_quality = ch->quality;

                ESP_LOGI(TAG, "[DATA] Flow: %.3f L/min, Quality: %d%%",
                         vpi_data.sensors.flow_lpm, ch->quality);
            }
        }

        // Update status
        vpi_data.status.sensor_online = true;
        vpi_data.status.last_sensor_rx = esp_log_timestamp();

        xSemaphoreGive(data_mutex);

        // Update VPIDocument if available
        if (vpi_document_handle != NULL)
        {
            pressure_data_t pressure_snapshot = {
                .current_ma = vpi_data.sensors.pressure_current_ma,
                .pressure_bar = vpi_data.sensors.pressure_bar,
                .quality = vpi_data.sensors.pressure_quality};
            vpi_document_update_pressure(vpi_document_handle, &pressure_snapshot);
        }

        // Update UI outside mutex (avoid blocking LVGL)
        // Send percentage to UI (0-100%)
        ui_vpi_hmi_update_pressure(vpi_data.sensors.pressure_percent);
    }
}

/**
 * @brief Node status callback
 */
static void on_node_status(const uart_node_status_t *status)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        // Update RSSI based on node type
        if (status->node_type == 0x10)
        { // PM2230
            vpi_data.status.pm2230_rssi = status->rssi;
        }
        else if (status->node_type == 0x20)
        { // 4-20mA
            vpi_data.status.sensor_rssi = status->rssi;
        }

        xSemaphoreGive(data_mutex);

        ESP_LOGD(TAG, "Node status: ID=%d, Type=0x%02X, RSSI=%d dBm, %s",
                 status->node_id, status->node_type, status->rssi,
                 status->is_online ? "ONLINE" : "OFFLINE");
    }
}

/**
 * @brief Gateway info callback
 */
static void on_gateway_info(const uart_gateway_info_t *info)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        vpi_data.system.total_packets = info->valid_packets;
        vpi_data.system.crc_errors = info->crc_errors;

        xSemaphoreGive(data_mutex);

        ESP_LOGI(TAG, "Gateway: Nodes=%d, Packets=%lu, CRC Errors=%lu, Uptime=%lus",
                 info->active_nodes, info->valid_packets,
                 info->crc_errors, info->uptime_seconds);
    }
}

// ============================================================================
// MONITOR TASK
// ============================================================================

/**
 * @brief Monitor task - checks data freshness and updates status
 */
static void monitor_task(void *arg)
{
    ESP_LOGI(TAG, "Monitor task started");

    uint32_t loop_count = 0;

    while (manager_running)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second

        uint32_t now = esp_log_timestamp();

        if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // Check PM2230 timeout
            if (vpi_data.status.pm2230_online)
            {
                if (now - vpi_data.status.last_pm2230_rx > 8000)
                {
                    vpi_data.status.pm2230_online = false;
                    vpi_data.pm2230.valid = false;
                    ESP_LOGW(TAG, "PM2230 node timeout");
                }
            }

            // Check sensor timeout
            if (vpi_data.status.sensor_online)
            {
                if (now - vpi_data.status.last_sensor_rx > 8000)
                {
                    vpi_data.status.sensor_online = false;
                    vpi_data.sensors.valid = false;
                    ESP_LOGW(TAG, "Sensor node timeout");
                }
            }

            // Check UART connection
            vpi_data.system.uart_connected = uart_receiver_is_active();

            // Update system uptime
            vpi_data.system.system_uptime = now / 1000;

            xSemaphoreGive(data_mutex);
        }

        // Log status every 10 seconds
        loop_count++;
        if (loop_count >= 10)
        {
            loop_count = 0;

            // Build status string
            char status_buf[128];
            snprintf(status_buf, sizeof(status_buf),
                     "PM2230: %s | Sensor: %s | UART: %s",
                     vpi_data.status.pm2230_online ? "ONLINE" : "OFFLINE",
                     vpi_data.status.sensor_online ? "ONLINE" : "OFFLINE",
                     vpi_data.system.uart_connected ? "CONNECTED" : "DISCONNECTED");

            ESP_LOGI(TAG, "Status: PM2230=%s, Sensor=%s, UART=%s",
                     vpi_data.status.pm2230_online ? "ONLINE" : "OFFLINE",
                     vpi_data.status.sensor_online ? "ONLINE" : "OFFLINE",
                     vpi_data.system.uart_connected ? "CONNECTED" : "DISCONNECTED");

            // Update UI status bar
            ui_vpi_hmi_update_status(status_buf);
        }
    }

    ESP_LOGI(TAG, "Monitor task stopped");
    monitor_task_handle = NULL;
    vTaskDelete(NULL);
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

esp_err_t vpi_data_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing VPI data manager");

    // Create mutex
    data_mutex = xSemaphoreCreateMutex();
    if (data_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create data mutex");
        return ESP_FAIL;
    }

    // Initialize data
    memset(&vpi_data, 0, sizeof(vpi_data_t));

    // Register UART callbacks
    uart_receiver_register_pm2230_callback(on_pm2230_data);
    uart_receiver_register_4_20ma_callback(on_4_20ma_data);
    uart_receiver_register_node_status_callback(on_node_status);
    uart_receiver_register_gateway_info_callback(on_gateway_info);

    ESP_LOGI(TAG, "VPI data manager initialized");
    return ESP_OK;
}

esp_err_t vpi_data_manager_start(void)
{
    if (manager_running)
    {
        ESP_LOGW(TAG, "Data manager already running");
        return ESP_ERR_INVALID_STATE;
    }

    manager_running = true;

    // Create monitor task
    BaseType_t ret = xTaskCreate(monitor_task,
                                 "vpi_monitor",
                                 3072,
                                 NULL,
                                 4,
                                 &monitor_task_handle);

    if (ret != pdPASS)
    {
        manager_running = false;
        ESP_LOGE(TAG, "Failed to create monitor task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "VPI data manager started");
    return ESP_OK;
}

const vpi_data_t *vpi_data_get(void)
{
    return &vpi_data;
}

bool vpi_data_is_pm2230_valid(void)
{
    uint32_t now = esp_log_timestamp();
    return vpi_data.pm2230.valid && (now - vpi_data.pm2230.last_update) < 5000;
}

bool vpi_data_is_sensor_valid(void)
{
    uint32_t now = esp_log_timestamp();
    return vpi_data.sensors.valid && (now - vpi_data.sensors.last_update) < 5000;
}

uint32_t vpi_data_get_age_ms(int data_type)
{
    uint32_t now = esp_log_timestamp();

    if (data_type == 0)
    { // PM2230
        return now - vpi_data.pm2230.last_update;
    }
    else
    { // Sensor
        return now - vpi_data.sensors.last_update;
    }
}

void vpi_data_manager_set_document(vpi_document_handle_t handle)
{
    vpi_document_handle = handle;
    ESP_LOGI(TAG, "VPIDocument handle set for data logging");
}
