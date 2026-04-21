#ifndef VPI_DATA_SNAPSHOT_H
#define VPI_DATA_SNAPSHOT_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief VPI System States
 */
typedef enum {
    VPI_STATE_IDLE = 0,
    VPI_STATE_VACUUM,
    VPI_STATE_OVERPRESSURE,
    VPI_STATE_LEAKING,
    VPI_STATE_COOLDOWN,
    VPI_STATE_ERROR
} vpi_state_t;

/**
 * @brief PM2230 Power Meter Data
 */
typedef struct {
    float voltage_a;    // Phase A voltage (V)
    float voltage_b;    // Phase B voltage (V)
    float voltage_c;    // Phase C voltage (V)
    float current_a;    // Phase A current (A)
    float current_b;    // Phase B current (A)
    float current_c;    // Phase C current (A)
    uint32_t power;     // Total power (W)
} pm2230_data_t;

/**
 * @brief 4-20mA Pressure Sensor Data
 */
typedef struct {
    float current_ma;   // Current reading (mA)
    float pressure_bar; // Pressure (bar)
    uint8_t quality;    // Signal quality (0-100%)
} pressure_data_t;

/**
 * @brief Complete VPI System Data Snapshot
 * 
 * Immutable snapshot of all system data at a specific timestamp.
 * Used for logging, observer notifications, and data persistence.
 */
typedef struct {
    time_t timestamp;           // Unix timestamp
    vpi_state_t state;          // Current VPI state
    pm2230_data_t pm2230;       // Power meter data
    pressure_data_t pressure;   // Pressure sensor data
    bool data_valid;            // Data validity flag
} vpi_data_snapshot_t;

/**
 * @brief Get state name as string
 * @param state VPI state
 * @return Constant string with state name
 */
const char* vpi_state_to_string(vpi_state_t state);

#ifdef __cplusplus
}
#endif

#endif // VPI_DATA_SNAPSHOT_H
