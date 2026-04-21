/*
 * VPI HMI User Interface
 * Landscape UI: 1024x600 with Electrical (left) and Tank (right) panels
 */

#ifndef UI_VPI_HMI_H
#define UI_VPI_HMI_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create VPI HMI main screen
 */
void ui_vpi_hmi_create(void);

/**
 * @brief Update VPI HMI with current data
 * 
 * Call this periodically to refresh displayed values
 */
void ui_vpi_hmi_update(void);

/**
 * @brief Update electrical parameters for a specific phase
 * 
 * @param phase Phase number (1, 2, or 3)
 * @param voltage Voltage in V
 * @param current Current in A
 * @param power Active power in W
 * @param reactive Reactive power in VAR
 * @param energy Energy in kWh
 */
void ui_vpi_hmi_update_electrical(uint8_t phase, float voltage, float current, 
                                   float power, float reactive, float energy);

/**
 * @brief Update tank pressure display
 * 
 * @param percentage Pressure percentage (0-100%, from 4-20mA standard)
 */
void ui_vpi_hmi_update_pressure(float percentage);

/**
 * @brief Update status bar message
 * 
 * @param status Status message string
 */
void ui_vpi_hmi_update_status(const char *status);

#ifdef __cplusplus
}
#endif

#endif // UI_VPI_HMI_H
