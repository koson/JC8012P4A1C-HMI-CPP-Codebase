/**
 * @file vpi_data_manager.h
 * @brief VPI Data Manager - Manages sensor data and updates UI
 */

#ifndef VPI_DATA_MANAGER_H
#define VPI_DATA_MANAGER_H

#include "espnow_uart_bridge.h"
#include "uart_receiver.h"
#include "VPIDocumentCWrapper.h"  // VPIDocument C wrapper
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief VPI system data (latest values)
 */
typedef struct {
    // PM2230 Power Meter Data
    struct {
        bool     valid;                 ///< Data validity flag
        uint32_t last_update;           ///< Last update timestamp (ms)
        uint8_t  meter_id;              ///< Meter ID
        
        float    voltage_an;            ///< Phase A-N voltage (V)
        float    voltage_bn;            ///< Phase B-N voltage (V)
        float    voltage_cn;            ///< Phase C-N voltage (V)
        
        float    current_a;             ///< Phase A current (A)
        float    current_b;             ///< Phase B current (A)
        float    current_c;             ///< Phase C current (A)
        
        float    power_active;          ///< Active power (W)
        float    power_reactive;        ///< Reactive power (VAR)
        float    power_apparent;        ///< Apparent power (VA)
        float    power_factor;          ///< Power factor
        
        float    energy_total;          ///< Total energy (kWh)
        float    frequency;             ///< Frequency (Hz)
        uint8_t  error_flags;           ///< Error flags
    } pm2230;
    
    // 4-20mA Sensor Data (Pressure & Flow)
    struct {
        bool     valid;                 ///< Data validity flag
        uint32_t last_update;           ///< Last update timestamp (ms)
        
        float    pressure_current_ma;   ///< Pressure sensor current (mA)
        float    pressure_percent;      ///< Pressure percentage (0-100%, from 4-20mA)
        float    pressure_bar;          ///< Pressure (bar) - from sensor datasheet
        float    pressure_psi;          ///< Pressure (PSI) - from sensor datasheet
        uint8_t  pressure_quality;      ///< Signal quality (0-100%)
        
        float    flow_lpm;              ///< Flow rate (L/min)
        uint8_t  flow_quality;          ///< Signal quality (0-100%)
        
        uint8_t  adc_status;            ///< ADC status
    } sensors;
    
    // Node Status
    struct {
        bool     pm2230_online;         ///< PM2230 node online
        bool     sensor_online;         ///< 4-20mA node online
        int8_t   pm2230_rssi;           ///< PM2230 RSSI (dBm)
        int8_t   sensor_rssi;           ///< Sensor RSSI (dBm)
        uint32_t last_pm2230_rx;        ///< Last PM2230 RX time (ms)
        uint32_t last_sensor_rx;        ///< Last sensor RX time (ms)
    } status;
    
    // System Status
    struct {
        bool     uart_connected;        ///< UART connection status
        uint32_t total_packets;         ///< Total packets received
        uint32_t crc_errors;            ///< CRC errors
        uint32_t system_uptime;         ///< System uptime (s)
    } system;
    
} vpi_data_t;

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

/**
 * @brief Initialize VPI data manager
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t vpi_data_manager_init(void);

/**
 * @brief Start VPI data manager
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t vpi_data_manager_start(void);

/**
 * @brief Get pointer to VPI data (thread-safe read)
 * 
 * @return const vpi_data_t* Pointer to VPI data
 */
const vpi_data_t* vpi_data_get(void);

/**
 * @brief Check if PM2230 data is valid
 * 
 * @return true if data is fresh (< 5 seconds old)
 */
bool vpi_data_is_pm2230_valid(void);

/**
 * @brief Check if sensor data is valid
 * 
 * @return true if data is fresh (< 5 seconds old)
 */
bool vpi_data_is_sensor_valid(void);

/**
 * @brief Get data age in milliseconds
 * 
 * @param data_type 0=PM2230, 1=Sensor
 * @return uint32_t Age in milliseconds
 */
uint32_t vpi_data_get_age_ms(int data_type);

/**
 * @brief Set VPIDocument handle for data logging
 * 
 * @param handle VPIDocument handle
 */
void vpi_data_manager_set_document(vpi_document_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // VPI_DATA_MANAGER_H
