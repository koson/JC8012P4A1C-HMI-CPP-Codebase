/**
 * @file espnow_uart_bridge.h
 * @brief UART Bridge Protocol for ESP-NOW Gateway to ESP32-P4 HMI
 * 
 * This protocol bridges ESP-NOW sensor data to ESP32-P4 HMI via UART.
 * 
 * Data Flow:
 * [Sensor Nodes] --ESP-NOW--> [Gateway ESP32C6] --UART--> [ESP32-P4 HMI] --LVGL--> [Display]
 * 
 * UART Configuration:
 * - Baud Rate: 921600 bps
 * - Data Bits: 8
 * - Stop Bits: 1  
 * - Parity: None
 * - Flow Control: None
 * - TX Pin (Gateway): GPIO 4
 * - RX Pin (Gateway): GPIO 5
 * - TX Pin (ESP32-P4): GPIO 16
 * - RX Pin (ESP32-P4): GPIO 15
 * 
 * @version 1.0
 * @date 2026-04-17
 */

#ifndef ESPNOW_UART_BRIDGE_H
#define ESPNOW_UART_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// PROTOCOL VERSION
// ============================================================================

#define UART_BRIDGE_PROTOCOL_VERSION    0x01

// ============================================================================
// UART PACKET STRUCTURE
// ============================================================================

#define UART_START_MARKER      0xAA55      // Start of frame marker
#define UART_END_MARKER        0x55AA      // End of frame marker

#define UART_HEADER_SIZE       10          // Header size in bytes
#define UART_FOOTER_SIZE       4           // Footer size in bytes (CRC16 + End marker)
#define UART_MAX_PAYLOAD_SIZE  256         // Maximum payload size
#define UART_MAX_PACKET_SIZE   (UART_HEADER_SIZE + UART_MAX_PAYLOAD_SIZE + UART_FOOTER_SIZE)

// ============================================================================
// PACKET TYPES
// ============================================================================

/**
 * @brief UART packet type definitions
 */
typedef enum {
    UART_PKT_TYPE_PM2230        = 0x10,     ///< PM2230 power meter data
    UART_PKT_TYPE_4_20MA        = 0x20,     ///< 4-20mA sensor data
    UART_PKT_TYPE_NODE_STATUS   = 0x30,     ///< Node status update
    UART_PKT_TYPE_GATEWAY_INFO  = 0x40,     ///< Gateway information
    UART_PKT_TYPE_ERROR         = 0xE0,     ///< Error message
    UART_PKT_TYPE_HEARTBEAT     = 0xF0,     ///< Heartbeat/Keep-alive
} uart_packet_type_t;

// ============================================================================
// UART PACKET HEADER
// ============================================================================

/**
 * @brief UART packet header (10 bytes)
 */
typedef struct __attribute__((packed)) {
    uint16_t start_marker;          ///< Start marker (0xAA55)
    uint8_t  protocol_version;      ///< Protocol version
    uint8_t  packet_type;           ///< Packet type (see uart_packet_type_t)
    uint8_t  source_node_id;        ///< Source node ID (from ESP-NOW)
    uint8_t  sequence;              ///< Packet sequence number
    uint16_t payload_length;        ///< Payload data length
    uint16_t reserved;              ///< Reserved for future use
} uart_packet_header_t;

// ============================================================================
// UART PACKET FOOTER
// ============================================================================

/**
 * @brief UART packet footer (4 bytes)
 */
typedef struct __attribute__((packed)) {
    uint16_t crc16;                 ///< CRC-16/MODBUS checksum
    uint16_t end_marker;            ///< End marker (0x55AA)
} uart_packet_footer_t;

// ============================================================================
// PM2230 DATA PAYLOAD
// ============================================================================

/**
 * @brief PM2230 power meter data (62 bytes)
 * Same structure as in espnow_protocol.h
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;             ///< Unix timestamp or uptime
    uint8_t  meter_id;              ///< Meter slave ID
    
    // Voltage (12 bytes)
    float voltage_an;               ///< Phase A-N voltage (V)
    float voltage_bn;               ///< Phase B-N voltage (V)
    float voltage_cn;               ///< Phase C-N voltage (V)
    
    // Current (12 bytes)
    float current_a;                ///< Phase A current (A)
    float current_b;                ///< Phase B current (A)
    float current_c;                ///< Phase C current (A)
    
    // Power (16 bytes)
    float power_active;             ///< Active power (W)
    float power_reactive;           ///< Reactive power (VAR)
    float power_apparent;           ///< Apparent power (VA)
    float power_factor;             ///< Power factor (0.0-1.0)
    
    // Energy (12 bytes)
    float energy_delivered;         ///< Energy delivered (kWh)
    float energy_received;          ///< Energy received (kWh)
    float energy_total;             ///< Total energy (kWh)
    
    // Frequency (4 bytes)
    float frequency;                ///< Line frequency (Hz)
    
    // Status (1 byte)
    uint8_t error_flags;            ///< Error flags
} uart_pm2230_data_t;

// ============================================================================
// 4-20mA SENSOR DATA PAYLOAD
// ============================================================================

/**
 * @brief 4-20mA sensor type
 */
typedef enum {
    SENSOR_TYPE_PRESSURE    = 0x01,     ///< Pressure sensor
    SENSOR_TYPE_FLOW        = 0x02,     ///< Flow rate sensor
    SENSOR_TYPE_TEMPERATURE = 0x03,     ///< Temperature sensor
    SENSOR_TYPE_LEVEL       = 0x04,     ///< Level sensor
    SENSOR_TYPE_GENERIC     = 0xFF,     ///< Generic sensor
} sensor_type_t;

/**
 * @brief Single 4-20mA channel data (36 bytes)
 */
typedef struct __attribute__((packed)) {
    uint8_t  channel;               ///< ADC channel (0-3)
    uint8_t  sensor_type;           ///< Sensor type
    float    current_ma;            ///< Measured current (mA)
    float    voltage_mv;            ///< Measured voltage (mV)
    float    sensor_value;          ///< Scaled engineering value
    float    scale_min;             ///< Minimum scale
    float    scale_max;             ///< Maximum scale
    char     unit[8];               ///< Unit string
    uint8_t  error_flags;           ///< Error flags
    uint8_t  quality;               ///< Signal quality (0-100%)
} uart_4_20ma_channel_t;

/**
 * @brief 4-20mA sensor data payload (up to 4 channels)
 */
typedef struct __attribute__((packed)) {
    uint8_t                num_channels;    ///< Number of channels (1-4)
    uart_4_20ma_channel_t  channels[4];     ///< Channel data
    uint8_t                adc_status;      ///< ADC status
} uart_4_20ma_data_t;

// ============================================================================
// NODE STATUS PAYLOAD
// ============================================================================

/**
 * @brief Node status data (16 bytes)
 */
typedef struct __attribute__((packed)) {
    uint8_t  node_id;               ///< Node ID
    uint8_t  node_type;             ///< Node type (PM2230, 4-20mA, etc.)
    uint8_t  mac_addr[6];           ///< Node MAC address
    int8_t   rssi;                  ///< RSSI (dBm)
    uint8_t  battery_level;         ///< Battery level (0-100%)
    uint8_t  is_online;             ///< 1=online, 0=offline
    uint32_t uptime_seconds;        ///< Node uptime
    uint8_t  error_count;           ///< Error count
    uint8_t  reserved;              ///< Reserved
} uart_node_status_t;

// ============================================================================
// GATEWAY INFO PAYLOAD
// ============================================================================

/**
 * @brief Gateway information (32 bytes)
 */
typedef struct __attribute__((packed)) {
    uint8_t  gateway_mac[6];        ///< Gateway MAC address
    uint8_t  wifi_channel;          ///< WiFi channel
    uint8_t  active_nodes;          ///< Number of active nodes
    uint32_t total_packets_rx;      ///< Total packets received
    uint32_t valid_packets;         ///< Valid packets
    uint32_t crc_errors;            ///< CRC errors
    uint32_t uptime_seconds;        ///< Gateway uptime
    uint8_t  reserved[8];           ///< Reserved
} uart_gateway_info_t;

// ============================================================================
// HEARTBEAT PAYLOAD
// ============================================================================

/**
 * @brief Heartbeat payload (4 bytes)
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;             ///< Timestamp (ms)
} uart_heartbeat_t;

// ============================================================================
// CRC-16 CALCULATION
// ============================================================================

/**
 * @brief Calculate CRC-16/MODBUS
 * 
 * @param data Pointer to data buffer
 * @param length Length of data
 * @return uint16_t CRC-16 checksum
 */
static inline uint16_t uart_calc_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief Verify CRC-16 of received packet
 * 
 * @param packet Pointer to complete packet
 * @param length Total packet length
 * @return true if CRC is valid
 */
static inline bool uart_verify_crc16(const uint8_t *packet, uint16_t length)
{
    if (length < UART_HEADER_SIZE + UART_FOOTER_SIZE) {
        return false;
    }
    
    // CRC covers header + payload
    uint16_t crc_calc = uart_calc_crc16(packet, length - UART_FOOTER_SIZE);
    
    // Extract CRC from footer
    const uart_packet_footer_t *footer = (const uart_packet_footer_t *)(packet + length - UART_FOOTER_SIZE);
    
    return (crc_calc == footer->crc16);
}

/**
 * @brief Build UART packet
 * 
 * @param buffer Output buffer (must be >= UART_MAX_PACKET_SIZE)
 * @param packet_type Packet type
 * @param source_node_id Source node ID
 * @param sequence Sequence number
 * @param payload Payload data
 * @param payload_length Payload length
 * @return uint16_t Total packet length (0 if error)
 */
static inline uint16_t uart_build_packet(
    uint8_t *buffer,
    uart_packet_type_t packet_type,
    uint8_t source_node_id,
    uint8_t sequence,
    const void *payload,
    uint16_t payload_length)
{
    if (payload_length > UART_MAX_PAYLOAD_SIZE) {
        return 0;
    }
    
    // Build header
    uart_packet_header_t *header = (uart_packet_header_t *)buffer;
    header->start_marker = UART_START_MARKER;
    header->protocol_version = UART_BRIDGE_PROTOCOL_VERSION;
    header->packet_type = packet_type;
    header->source_node_id = source_node_id;
    header->sequence = sequence;
    header->payload_length = payload_length;
    header->reserved = 0;
    
    // Copy payload
    if (payload_length > 0 && payload != NULL) {
        memcpy(buffer + UART_HEADER_SIZE, payload, payload_length);
    }
    
    // Calculate CRC (header + payload)
    uint16_t crc = uart_calc_crc16(buffer, UART_HEADER_SIZE + payload_length);
    
    // Build footer
    uart_packet_footer_t *footer = (uart_packet_footer_t *)(buffer + UART_HEADER_SIZE + payload_length);
    footer->crc16 = crc;
    footer->end_marker = UART_END_MARKER;
    
    return UART_HEADER_SIZE + payload_length + UART_FOOTER_SIZE;
}

#ifdef __cplusplus
}
#endif

#endif // ESPNOW_UART_BRIDGE_H
