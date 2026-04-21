/**
 * @file VPITankGauge.h
 * @brief VPI Tank Pressure Gauge Widget (C++ OOP wrapper for LVGL v9)
 * 
 * Provides a round pressure gauge (0-100%) with image needle rotation
 */

#ifndef VPI_TANK_GAUGE_H
#define VPI_TANK_GAUGE_H

#include "lvgl.h"
#include <string>

/**
 * @brief VPI Tank Pressure Gauge Class
 * 
 * Features:
 * - Round scale gauge (0-100%, 270° arc)
 * - Image needle with smooth rotation
 * - Digital pressure readout below gauge
 * - Configurable colors and styles
 */
class VPITankGauge
{
public:
    /**
     * @brief Construct a new VPITankGauge
     * 
     * @param parent Parent container
     * @param needle_img Pointer to needle image descriptor (e.g., &img_pressure_needle)
     */
    VPITankGauge(lv_obj_t* parent, const lv_image_dsc_t* needle_img);
    
    /**
     * @brief Destroy the VPITankGauge
     */
    virtual ~VPITankGauge();
    
    /**
     * @brief Update pressure value
     * 
     * @param percentage Pressure percentage (0-100%)
     */
    void updatePressure(float percentage);
    
    /**
     * @brief Set gauge position
     * 
     * @param x X coordinate
     * @param y Y coordinate
     */
    void setPosition(int x, int y);
    
    /**
     * @brief Set gauge size
     * 
     * @param width Gauge diameter
     */
    void setSize(int width);
    
    /**
     * @brief Get current pressure value
     * 
     * @return float Current pressure percentage
     */
    float getPressure() const { return current_pressure_; }
    
private:
    lv_obj_t* gauge_;              ///< Scale widget (gauge)
    lv_obj_t* needle_;             ///< Image needle
    lv_obj_t* pressure_label_;     ///< Digital pressure readout
    
    float current_pressure_;       ///< Current pressure value (%)
    
    // Styling
    void applyStyles();
    void createDigitalReadout();
};

#endif // VPI_TANK_GAUGE_H
