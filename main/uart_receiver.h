/**
 * @file uart_receiver.h
 * @brief UART Receiver for ESP32-P4 HMI
 * 
 * Receives sensor data packets from Gateway ESP32C6 via UART
 */

#ifndef UART_RECEIVER_H
#define UART_RECEIVER_H

#include "espnow_uart_bridge.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONFIGURATION
// ============================================================================

// Gateway UART Communication (verified by continuity test)
#define UART_PORT_NUM           0           // UART0 (Gateway communication)
#define UART_BAUD_RATE          115200      // 115.2 kbps
#define UART_TX_PIN             37          // GPIO37 (UART0_TXD) - verified by continuity test
#define UART_RX_PIN             38          // GPIO38 (UART0_RXD) - verified by continuity test
#define UART_RX_BUFFER_SIZE     2048        // 2KB RX buffer
#define UART_TX_BUFFER_SIZE     1024        // 1KB TX buffer

// ============================================================================
// CALLBACK TYPES
// ============================================================================

/**
 * @brief Callback function types for different packet types
 */
typedef void (*uart_pm2230_callback_t)(uint8_t node_id, const uart_pm2230_data_t *data);
typedef void (*uart_4_20ma_callback_t)(uint8_t node_id, const uart_4_20ma_data_t *data);
typedef void (*uart_node_status_callback_t)(const uart_node_status_t *status);
typedef void (*uart_gateway_info_callback_t)(const uart_gateway_info_t *info);

/**
 * @brief UART receiver statistics
 */
typedef struct {
    uint32_t total_packets_rx;      ///< Total packets received
    uint32_t valid_packets;         ///< Valid packets
    uint32_t crc_errors;            ///< CRC errors
    uint32_t sync_errors;           ///< Sync/framing errors
    uint32_t timeout_errors;        ///< Timeout errors
    uint32_t last_rx_timestamp;     ///< Last RX timestamp (ms)
} uart_rx_stats_t;

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

/**
 * @brief Initialize UART receiver
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t uart_receiver_init(void);

/**
 * @brief Start UART receiver task
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t uart_receiver_start(void);

/**
 * @brief Stop UART receiver task
 */
void uart_receiver_stop(void);

/**
 * @brief Register PM2230 data callback
 * 
 * @param callback Callback function
 */
void uart_receiver_register_pm2230_callback(uart_pm2230_callback_t callback);

/**
 * @brief Register 4-20mA data callback
 * 
 * @param callback Callback function
 */
void uart_receiver_register_4_20ma_callback(uart_4_20ma_callback_t callback);

/**
 * @brief Register node status callback
 * 
 * @param callback Callback function
 */
void uart_receiver_register_node_status_callback(uart_node_status_callback_t callback);

/**
 * @brief Register gateway info callback
 * 
 * @param callback Callback function
 */
void uart_receiver_register_gateway_info_callback(uart_gateway_info_callback_t callback);

/**
 * @brief Get UART receiver statistics
 * 
 * @param stats Pointer to statistics structure
 */
void uart_receiver_get_stats(uart_rx_stats_t *stats);

/**
 * @brief Check if UART is receiving data
 * 
 * @return true if data received within last 5 seconds
 */
bool uart_receiver_is_active(void);

#ifdef __cplusplus
}
#endif

#endif // UART_RECEIVER_H
