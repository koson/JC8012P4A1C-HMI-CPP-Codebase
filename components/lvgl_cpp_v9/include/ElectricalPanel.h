/**
 * @file ElectricalPanel.h
 * @brief 3-Phase Electrical Parameters Panel (C++ OOP wrapper for LVGL v9)
 * 
 * Displays voltage, current, power, reactive power, and energy for 3 phases
 */

#ifndef ELECTRICAL_PANEL_H
#define ELECTRICAL_PANEL_H

#include "lvgl.h"
#include <string>

/**
 * @brief Single Phase Data Display
 */
struct PhaseLabels {
    lv_obj_t* voltage;     ///< Voltage label
    lv_obj_t* current;     ///< Current label
    lv_obj_t* power;       ///< Active power label
    lv_obj_t* reactive;    ///< Reactive power label
    lv_obj_t* energy;      ///< Energy label
};

/**
 * @brief 3-Phase Electrical Parameters Panel
 * 
 * Features:
 * - 3 phases (Yellow, Cyan, Magenta colors)
 * - Displays: Voltage (V), Current (A), Power (W), Reactive (VAR), Energy (kWh)
 * - Row labels on left side
 * - Thread-safe updates with display locking
 */
class ElectricalPanel
{
public:
    /**
     * @brief Construct a new ElectricalPanel
     * 
     * @param parent Parent screen
     */
    ElectricalPanel(lv_obj_t* parent);
    
    /**
     * @brief Destroy the ElectricalPanel
     */
    virtual ~ElectricalPanel();
    
    /**
     * @brief Update single phase data
     * 
     * @param phase Phase number (1, 2, or 3)
     * @param voltage Voltage (V)
     * @param current Current (A)
     * @param power Active power (W)
     * @param reactive Reactive power (VAR)
     * @param energy Total energy (kWh)
     */
    void updatePhase(uint8_t phase, float voltage, float current, 
                     float power, float reactive, float energy);
    
    /**
     * @brief Update all 3 phases at once
     * 
     * @param v1 Phase 1 voltage
     * @param v2 Phase 2 voltage
     * @param v3 Phase 3 voltage
     * @param a1 Phase 1 current
     * @param a2 Phase 2 current
     * @param a3 Phase 3 current
     * @param w1 Phase 1 power
     * @param w2 Phase 2 power
     * @param w3 Phase 3 power
     * @param var1 Phase 1 reactive
     * @param var2 Phase 2 reactive
     * @param var3 Phase 3 reactive
     * @param kwh1 Phase 1 energy
     * @param kwh2 Phase 2 energy
     * @param kwh3 Phase 3 energy
     */
    void updateAll(float v1, float v2, float v3,
                   float a1, float a2, float a3,
                   float w1, float w2, float w3,
                   float var1, float var2, float var3,
                   float kwh1, float kwh2, float kwh3);
    
    /**
     * @brief Set panel position
     * 
     * @param x X coordinate
     * @param y Y coordinate
     */
    void setPosition(int x, int y);
    
    /**
     * @brief Set panel size
     * 
     * @param width Panel width
     * @param height Panel height
     */
    void setSize(int width, int height);
    
private:
    lv_obj_t* panel_;          ///< Main panel container
    lv_obj_t* title_;          ///< Title label
    
    PhaseLabels phase1_;       ///< Phase 1 labels
    PhaseLabels phase2_;       ///< Phase 2 labels
    PhaseLabels phase3_;       ///< Phase 3 labels
    
    // Helper functions
    void createTitle();
    void createRowLabels();
    void createPhaseColumn(const char* phase_name, uint32_t color, 
                          int x_offset, PhaseLabels& labels);
    void updatePhaseLabels(const PhaseLabels& labels, 
                          float voltage, float current, 
                          float power, float reactive, float energy);
};

#endif // ELECTRICAL_PANEL_H
