/**
 * @file VPIHMIUI.h
 * @brief VPI HMI Main User Interface (C++ OOP)
 * 
 * Integrates Electrical Panel + VPI Tank Gauge + Status Bar
 * Layout: Landscape 1280x800 (rotated from 800x1280 portrait)
 */

#ifndef VPI_HMI_UI_H
#define VPI_HMI_UI_H

#include "lvgl.h"
#include "ElectricalPanel.h"
#include "VPITankGauge.h"
#include <string>

/**
 * @brief VPI HMI Main UI Class
 * 
 * Features:
 * - Left panel (800px): Electrical parameters (3-phase)
 * - Right panel (440px): VPI tank gauge
 * - Bottom status bar
 * - Landscape 1280×800 layout
 */
class VPIHMIUI
{
public:
    /**
     * @brief Construct a new VPIHMIUI
     * 
     * Creates UI on current active screen
     */
    VPIHMIUI();
    
    /**
     * @brief Destroy the VPIHMIUI
     */
    ~VPIHMIUI();
    
    /**
     * @brief Update electrical parameters for one phase
     * 
     * @param phase Phase number (1, 2, or 3)
     * @param voltage Voltage (V)
     * @param current Current (A)
     * @param power Active power (W)
     * @param reactive Reactive power (VAR)
     * @param energy Total energy (kWh)
     */
    void updateElectrical(uint8_t phase, float voltage, float current,
                         float power, float reactive, float energy);
    
    /**
     * @brief Update pressure gauge
     * 
     * @param percentage Pressure percentage (0-100%)
     */
    void updatePressure(float percentage);
    
    /**
     * @brief Update status bar message
     * 
     * @param status Status message string
     */
    void updateStatus(const char* status);
    
    /**
     * @brief Update date/time display
     * 
     * @param datetime DateTime string (e.g., "2026-04-21 14:35:22")
     */
    void updateDateTime(const char* datetime);
    
    /**
     * @brief Set LOG button state
     * 
     * @param is_logging true if logging active
     */
    void setLoggingState(bool is_logging);
    
    /**
     * @brief Get LOG button object (for event handling)
     */
    lv_obj_t* getLogButton() { return log_button_; }
    
    /**
     * @brief Get electrical panel
     */
    ElectricalPanel* getElectricalPanel() { return electrical_panel_; }
    
    /**
     * @brief Get tank gauge
     */
    VPITankGauge* getTankGauge() { return tank_gauge_; }
    
private:
    lv_obj_t* screen_;                ///< Main screen
    
    // Header bar
    lv_obj_t* header_panel_;          ///< Header bar panel
    lv_obj_t* title_label_;           ///< "VPI SYSTEM" title
    lv_obj_t* log_button_;            ///< LOG START/STOP button
    lv_obj_t* datetime_label_;        ///< Date/Time display
    
    // Main content
    ElectricalPanel* electrical_panel_;  ///< Electrical parameters panel
    lv_obj_t* tank_panel_;            ///< Tank panel container
    VPITankGauge* tank_gauge_;        ///< Pressure gauge
    lv_obj_t* status_label_;          ///< Status bar label
    
    // UI creation helpers
    void createHeaderBar();
    void createTankPanel();
    void createStatusBar();
};

#endif // VPI_HMI_UI_H
