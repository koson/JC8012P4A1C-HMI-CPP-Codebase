/**
 * @file uart_receiver.c
 * @brief UART Receiver Implementation for ESP32-P4 HMI
 */

#include "uart_receiver.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include <string.h>
#include <ctype.h>

static const char *TAG = "UART_RX";

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static TaskHandle_t uart_rx_task_handle = NULL;
static bool uart_rx_running = false;
static uart_rx_stats_t rx_stats = {0};

// Callbacks
static uart_pm2230_callback_t pm2230_callback = NULL;
static uart_4_20ma_callback_t sensor_4_20ma_callback = NULL;
static uart_node_status_callback_t node_status_callback = NULL;
static uart_gateway_info_callback_t gateway_info_callback = NULL;

// RX buffer - larger for JSON mode (PM2230 ~400 bytes + 4-20mA ~200 bytes)
#define UART_JSON_BUFFER_SIZE  2048
static uint8_t uart_rx_buffer[UART_JSON_BUFFER_SIZE];

// ============================================================================
// UART INITIALIZATION
// ============================================================================

esp_err_t uart_receiver_init(void)
{
    // Static flag to prevent multiple initialization
    static bool initialized = false;
    if (initialized) {
        ESP_LOGW(TAG, "UART receiver already initialized, skipping...");
        return ESP_OK;
    }
    
    // UART configuration
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_LOGI(TAG, "Initializing UART%d: %d bps, TX=GPIO%d, RX=GPIO%d",
             UART_PORT_NUM, UART_BAUD_RATE, UART_TX_PIN, UART_RX_PIN);
    
    // Check if UART driver is already installed
    esp_err_t ret = uart_driver_install(UART_PORT_NUM, 
                                         UART_RX_BUFFER_SIZE,
                                         UART_TX_BUFFER_SIZE, 
                                         0, NULL, 0);
    if (ret == ESP_FAIL) {
        ESP_LOGW(TAG, "UART driver already installed, skipping installation");
    } else {
        ESP_ERROR_CHECK(ret);
    }
    
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    
    // Set UART pins
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, 
                                  UART_TX_PIN, 
                                  UART_RX_PIN,
                                  UART_PIN_NO_CHANGE, 
                                  UART_PIN_NO_CHANGE));
    
    initialized = true;
    ESP_LOGI(TAG, "UART initialized successfully");
    return ESP_OK;
}

// ============================================================================
// JSON PACKET PROCESSING
// ============================================================================

/**
 * @brief Process received JSON packet from Gateway
 */
static void process_json_packet(const char *json_str, size_t length)
{
    if (json_str == NULL || length == 0) {
        return;
    }
    
    cJSON *root = cJSON_ParseWithLength(json_str, length);
    if (root == NULL) {
        ESP_LOGW(TAG, "JSON parse error (%d bytes)", length);
        // Print first and last 50 chars to debug
        char preview[120];
        if (length > 100) {
            snprintf(preview, sizeof(preview), "%.50s ... %.50s", json_str, json_str + length - 50);
        } else {
            snprintf(preview, sizeof(preview), "%.*s", (int)length, json_str);
        }
        ESP_LOGW(TAG, "Failed JSON: %s", preview);
        rx_stats.sync_errors++;
        return;
    }
    
    // Update stats
    rx_stats.valid_packets++;
    rx_stats.last_rx_timestamp = esp_log_timestamp();
    
    // Get node_id
    cJSON *node_id_item = cJSON_GetObjectItem(root, "node_id");
    uint8_t node_id = (node_id_item && cJSON_IsNumber(node_id_item)) ? node_id_item->valueint : 0;
    
    // Get type
    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    if (type_item == NULL || !cJSON_IsString(type_item)) {
        ESP_LOGW(TAG, "Missing or invalid type field");
        cJSON_Delete(root);
        return;
    }
    
    const char *type_str = type_item->valuestring;
    
    // Process based on type
    if (strcmp(type_str, "4-20mA") == 0) {
        // Parse 4-20mA sensor data
        if (sensor_4_20ma_callback != NULL) {
            uart_4_20ma_data_t data = {0};
            
            cJSON *channels = cJSON_GetObjectItem(root, "channels");
            if (channels && cJSON_IsArray(channels)) {
                int channel_count = cJSON_GetArraySize(channels);
                data.num_channels = (channel_count > 4) ? 4 : channel_count;
                
                for (int i = 0; i < data.num_channels; i++) {
                    cJSON *ch = cJSON_GetArrayItem(channels, i);
                    if (ch) {
                        cJSON *ch_num = cJSON_GetObjectItem(ch, "ch");
                        cJSON *ch_type = cJSON_GetObjectItem(ch, "type");
                        cJSON *current = cJSON_GetObjectItem(ch, "current");
                        cJSON *value = cJSON_GetObjectItem(ch, "value");
                        cJSON *unit = cJSON_GetObjectItem(ch, "unit");
                        cJSON *quality = cJSON_GetObjectItem(ch, "quality");
                        
                        if (ch_num) data.channels[i].channel = ch_num->valueint;
                        if (ch_type) data.channels[i].sensor_type = ch_type->valueint;
                        if (current) data.channels[i].current_ma = (float)current->valuedouble;
                        if (value) data.channels[i].sensor_value = (float)value->valuedouble;
                        if (quality) data.channels[i].quality = quality->valueint;
                        
                        if (unit && cJSON_IsString(unit)) {
                            strncpy(data.channels[i].unit, unit->valuestring, 
                                   sizeof(data.channels[i].unit) - 1);
                        }
                    }
                }
            }
            
            cJSON *adc_status = cJSON_GetObjectItem(root, "adc_status");
            if (adc_status) data.adc_status = adc_status->valueint;
            
            ESP_LOGI(TAG, "4-20mA data: Node=%d, Channels=%d", node_id, data.num_channels);
            for (int i = 0; i < data.num_channels; i++) {
                ESP_LOGI(TAG, "  Ch%d: %.2fmA = %.3f %s (Q:%d%%)", 
                         data.channels[i].channel,
                         data.channels[i].current_ma,
                         data.channels[i].sensor_value,
                         data.channels[i].unit,
                         data.channels[i].quality);
            }
            sensor_4_20ma_callback(node_id, &data);
        }
    }
    else if (strcmp(type_str, "PM2230") == 0) {
        // Parse PM2230 power meter data
        if (pm2230_callback != NULL) {
            uart_pm2230_data_t data = {0};
            
            // Parse voltage array [An, Bn, Cn]
            cJSON *voltage = cJSON_GetObjectItem(root, "voltage");
            if (voltage && cJSON_IsArray(voltage) && cJSON_GetArraySize(voltage) >= 3) {
                cJSON *v_an = cJSON_GetArrayItem(voltage, 0);
                cJSON *v_bn = cJSON_GetArrayItem(voltage, 1);
                cJSON *v_cn = cJSON_GetArrayItem(voltage, 2);
                if (v_an) data.voltage_an = (float)v_an->valuedouble;
                if (v_bn) data.voltage_bn = (float)v_bn->valuedouble;
                if (v_cn) data.voltage_cn = (float)v_cn->valuedouble;
            }
            
            // Parse current array [A, B, C]
            cJSON *current = cJSON_GetObjectItem(root, "current");
            if (current && cJSON_IsArray(current) && cJSON_GetArraySize(current) >= 3) {
                cJSON *i_a = cJSON_GetArrayItem(current, 0);
                cJSON *i_b = cJSON_GetArrayItem(current, 1);
                cJSON *i_c = cJSON_GetArrayItem(current, 2);
                if (i_a) data.current_a = (float)i_a->valuedouble;
                if (i_b) data.current_b = (float)i_b->valuedouble;
                if (i_c) data.current_c = (float)i_c->valuedouble;
            }
            
            // Parse power object
            cJSON *power = cJSON_GetObjectItem(root, "power");
            if (power) {
                cJSON *active = cJSON_GetObjectItem(power, "active");
                cJSON *reactive = cJSON_GetObjectItem(power, "reactive");
                cJSON *apparent = cJSON_GetObjectItem(power, "apparent");
                cJSON *pf = cJSON_GetObjectItem(power, "pf");
                if (active) data.power_active = (float)active->valuedouble;
                if (reactive) data.power_reactive = (float)reactive->valuedouble;
                if (apparent) data.power_apparent = (float)apparent->valuedouble;
                if (pf && !cJSON_IsNull(pf)) data.power_factor = (float)pf->valuedouble;
            }
            
            // Parse energy object
            cJSON *energy = cJSON_GetObjectItem(root, "energy");
            if (energy) {
                cJSON *delivered = cJSON_GetObjectItem(energy, "delivered");
                cJSON *received = cJSON_GetObjectItem(energy, "received");
                cJSON *total = cJSON_GetObjectItem(energy, "total");
                if (delivered) data.energy_delivered = (float)delivered->valuedouble;
                if (received) data.energy_received = (float)received->valuedouble;
                if (total) data.energy_total = (float)total->valuedouble;
            }
            
            // Parse frequency
            cJSON *freq = cJSON_GetObjectItem(root, "frequency");
            if (freq) data.frequency = (float)freq->valuedouble;
            
            // Parse error_flags
            cJSON *error_flags = cJSON_GetObjectItem(root, "error_flags");
            if (error_flags) data.error_flags = (uint8_t)error_flags->valueint;
            
            ESP_LOGI(TAG, "PM2230 data: Node=%d, V=%.1f/%.1f/%.1f", 
                     node_id, data.voltage_an, data.voltage_bn, data.voltage_cn);
            pm2230_callback(node_id, &data);
        }
    }
    
    cJSON_Delete(root);
}

// ============================================================================
// UART RECEIVE TASK
// ============================================================================

/**
 * @brief UART receive task - JSON mode
 */
static void uart_rx_task(void *arg)
{
    ESP_LOGI(TAG, "UART RX task started (JSON mode)");
    
    char *line_buffer = (char *)uart_rx_buffer;
    int rx_pos = 0;
    
    while (uart_rx_running) {
        // Read data from UART
        int len = uart_read_bytes(UART_PORT_NUM, 
                                   (uint8_t *)(line_buffer + rx_pos), 
                                   UART_JSON_BUFFER_SIZE - rx_pos - 1,  // Leave space for null terminator
                                   pdMS_TO_TICKS(100));
        
        if (len > 0) {
            rx_pos += len;
            line_buffer[rx_pos] = '\0';  // Null terminate
            rx_stats.total_packets_rx++;
            
            // Look for JSON objects with proper bracket matching
            char *json_start = strchr(line_buffer, '{');
            while (json_start != NULL) {
                // Find matching closing brace by counting depth
                char *json_end = NULL;
                int depth = 0;
                for (char *p = json_start; *p != '\0'; p++) {
                    if (*p == '{') depth++;
                    else if (*p == '}') {
                        depth--;
                        if (depth == 0) {
                            json_end = p;
                            break;
                        }
                    }
                }
                
                if (json_end != NULL) {
                    // Found complete JSON object with matching braces
                    size_t json_len = json_end - json_start + 1;
                    
                    // Process JSON packet
                    process_json_packet(json_start, json_len);
                    
                    // Remove processed JSON from buffer
                    int remaining = rx_pos - (json_end - line_buffer + 1);
                    if (remaining > 0) {
                        memmove(line_buffer, json_end + 1, remaining);
                        rx_pos = remaining;
                        line_buffer[rx_pos] = '\0';
                        json_start = strchr(line_buffer, '{');  // Look for next JSON
                    } else {
                        rx_pos = 0;
                        break;
                    }
                } else {
                    // Incomplete JSON, wait for more data
                    // But if json_start is not at the beginning, remove garbage before it
                    if (json_start != line_buffer) {
                        int offset = json_start - line_buffer;
                        memmove(line_buffer, json_start, rx_pos - offset);
                        rx_pos -= offset;
                        line_buffer[rx_pos] = '\0';
                    }
                    break;
                }
            }
            
            // If no JSON start found, clear buffer (garbage data)
            if (json_start == NULL && rx_pos > 0) {
                // Keep last 100 bytes in case JSON spans reads
                if (rx_pos > 100) {
                    memmove(line_buffer, line_buffer + rx_pos - 100, 100);
                    rx_pos = 100;
                }
            }
            
            // Prevent buffer overflow
            if (rx_pos >= UART_JSON_BUFFER_SIZE - 1) {
                ESP_LOGW(TAG, "Buffer overflow, resetting (pos=%d)", rx_pos);
                rx_pos = 0;
                rx_stats.sync_errors++;
            }
        }
    }
    
    ESP_LOGI(TAG, "UART RX task stopped");
    uart_rx_task_handle = NULL;
    vTaskDelete(NULL);
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

esp_err_t uart_receiver_start(void)
{
    if (uart_rx_running) {
        ESP_LOGW(TAG, "UART RX already running");
        return ESP_ERR_INVALID_STATE;
    }
    
    uart_rx_running = true;
    
    // Create UART RX task
    BaseType_t ret = xTaskCreate(uart_rx_task, 
                                  "uart_rx", 
                                  4096, 
                                  NULL, 
                                  5, 
                                  &uart_rx_task_handle);
    
    if (ret != pdPASS) {
        uart_rx_running = false;
        ESP_LOGE(TAG, "Failed to create UART RX task");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "UART RX task started");
    return ESP_OK;
}

void uart_receiver_stop(void)
{
    if (!uart_rx_running) {
        return;
    }
    
    uart_rx_running = false;
    
    // Wait for task to finish
    if (uart_rx_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    ESP_LOGI(TAG, "UART RX stopped");
}

void uart_receiver_register_pm2230_callback(uart_pm2230_callback_t callback)
{
    pm2230_callback = callback;
    ESP_LOGI(TAG, "PM2230 callback registered");
}

void uart_receiver_register_4_20ma_callback(uart_4_20ma_callback_t callback)
{
    sensor_4_20ma_callback = callback;
    ESP_LOGI(TAG, "4-20mA callback registered");
}

void uart_receiver_register_node_status_callback(uart_node_status_callback_t callback)
{
    node_status_callback = callback;
    ESP_LOGI(TAG, "Node status callback registered");
}

void uart_receiver_register_gateway_info_callback(uart_gateway_info_callback_t callback)
{
    gateway_info_callback = callback;
    ESP_LOGI(TAG, "Gateway info callback registered");
}

void uart_receiver_get_stats(uart_rx_stats_t *stats)
{
    if (stats != NULL) {
        memcpy(stats, &rx_stats, sizeof(uart_rx_stats_t));
    }
}

bool uart_receiver_is_active(void)
{
    uint32_t now = esp_log_timestamp();
    return (now - rx_stats.last_rx_timestamp) < 5000;  // 5 seconds
}
