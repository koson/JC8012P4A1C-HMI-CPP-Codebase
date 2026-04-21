/**
 * @file LVMeter.hpp
 * @brief C++ Wrapper สำหรับ LVGL Scale Widget (แทน Meter ที่ถูกลบใน v9)
 * 
 * Note: LVGL v9 ลบ Meter widget ออก - ใช้ Scale widget แทน
 * Scale = Circular/Linear gauge with ticks and range
 */

#ifndef LV_METER_HPP
#define LV_METER_HPP

#include "LVWidget.hpp"

/**
 * @class LVMeter
 * @brief Meter-style widget ใช้ Scale widget ภายใน (LVGL v9 compatible)
 */
class LVMeter : public LVWidget {
public:
    explicit LVMeter(LVWidget* parent = nullptr);

    // Basic scale configuration
    LVMeter* setRange(int32_t min, int32_t max);
    LVMeter* setAngleRange(uint32_t angle_range);
    LVMeter* setRotation(uint32_t rotation);
    
    // Ticks configuration
    LVMeter* setTotalTickCount(uint16_t count);
    LVMeter* setMajorTickEvery(uint16_t nth);
    
    // Appearance
    LVMeter* setTickLength(uint16_t major_len, uint16_t minor_len);
    LVMeter* setTickWidth(uint16_t major_width, uint16_t minor_width);
    
    // Value setting
    LVMeter* setValue(int32_t value);
    int32_t getValue() const;
    
    // Mode
    LVMeter* setMode(lv_scale_mode_t mode);  // ROUND_INNER, ROUND_OUTER, HORIZONTAL, VERTICAL
    
    // Section/Indicator (LVGL v9 scale uses sections for color zones)
    lv_scale_section_t* addSection(int32_t min, int32_t max, lv_color_t main_color, lv_color_t tick_color);
    lv_scale_section_t* addProgressSection(lv_color_t color);  // Auto-update section from min to value
    LVMeter* updateSection(lv_scale_section_t* section, int32_t min, int32_t max);
    LVMeter* removeSection(lv_scale_section_t* section);
    
    // Needle/Indicator using line (for round scales)
    LVMeter* enableLineNeedle(uint16_t length, uint16_t width, lv_color_t color);
    LVMeter* updateNeedle();
    
    // Progress arc (alternative to needle for round scales)
    LVMeter* enableProgressArc(uint16_t width, lv_color_t color);
    LVMeter* updateProgressArc();

private:
    int32_t m_value = 0;
    int32_t m_min = 0;
    int32_t m_max = 100;
    lv_scale_mode_t m_mode = LV_SCALE_MODE_ROUND_INNER;
    uint32_t m_angleRange = 270;
    uint32_t m_rotation = 135;
    
    // Needle/indicator objects
    lv_obj_t* m_needle = nullptr;
    lv_obj_t* m_progressArc = nullptr;
    lv_scale_section_t* m_progressSection = nullptr;
    uint16_t m_needleLength = 80;
    
    // Helper
    int32_t valueToAngle(int32_t value);
};

#endif // LV_METER_HPP
