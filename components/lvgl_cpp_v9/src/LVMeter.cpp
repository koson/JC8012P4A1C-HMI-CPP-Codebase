/**
 * @file LVMeter.cpp
 * @brief Implementation ของ LVMeter ใช้ Scale widget (LVGL v9)
 */

#include "../include/LVMeter.hpp"
#include "esp_log.h"
#include <cmath>

LVMeter::LVMeter(LVWidget* parent)
    : LVWidget(parent, lv_scale_create(parent ? parent->obj() : nullptr))
{
    // Default configuration for meter-like appearance
    m_mode = LV_SCALE_MODE_ROUND_INNER;
    lv_scale_set_mode(m_obj, m_mode);
    lv_obj_set_size(m_obj, 200, 200);
}

LVMeter* LVMeter::setRange(int32_t min, int32_t max)
{
    m_min = min;
    m_max = max;
    lv_scale_set_range(m_obj, min, max);
    return this;
}

LVMeter* LVMeter::setAngleRange(uint32_t angle_range)
{
    m_angleRange = angle_range;
    lv_scale_set_angle_range(m_obj, angle_range);
    return this;
}

LVMeter* LVMeter::setRotation(uint32_t rotation)
{
    m_rotation = rotation;
    lv_scale_set_rotation(m_obj, rotation);
    return this;
}

LVMeter* LVMeter::setTotalTickCount(uint16_t count)
{
    lv_scale_set_total_tick_count(m_obj, count);
    return this;
}

LVMeter* LVMeter::setMajorTickEvery(uint16_t nth)
{
    lv_scale_set_major_tick_every(m_obj, nth);
    return this;
}

LVMeter* LVMeter::setTickLength(uint16_t major_len, uint16_t minor_len)
{
    lv_obj_set_style_length(m_obj, major_len, LV_PART_INDICATOR);
    lv_obj_set_style_length(m_obj, minor_len, LV_PART_ITEMS);
    return this;
}

LVMeter* LVMeter::setTickWidth(uint16_t major_width, uint16_t minor_width)
{
    lv_obj_set_style_line_width(m_obj, major_width, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(m_obj, minor_width, LV_PART_ITEMS);
    return this;
}

LVMeter* LVMeter::setValue(int32_t value)
{
    m_value = value;
    ESP_LOGI("LVMeter", "setValue(%d) - needle:%p, arc:%p, section:%p", 
             (int)value, m_needle, m_progressArc, m_progressSection);
    
    // Update needle/arc/section if enabled
    if (m_needle) {
        updateNeedle();
        ESP_LOGI("LVMeter", "  → Needle updated");
    }
    if (m_progressArc) {
        updateProgressArc();
        ESP_LOGI("LVMeter", "  → Arc updated");
    }
    if (m_progressSection) {
        updateSection(m_progressSection, m_min, m_value);
        ESP_LOGI("LVMeter", "  → Section updated: range %d-%d", (int)m_min, (int)m_value);
    }
    return this;
}

int32_t LVMeter::getValue() const
{
    return m_value;
}

LVMeter* LVMeter::setMode(lv_scale_mode_t mode)
{
    m_mode = mode;
    lv_scale_set_mode(m_obj, mode);
    return this;
}

lv_scale_section_t* LVMeter::addSection(int32_t min, int32_t max, lv_color_t main_color, lv_color_t tick_color)
{
    lv_scale_section_t* section = lv_scale_add_section(m_obj);
    lv_scale_section_set_range(section, min, max);
    
    // Allocate new styles for each section (must persist)
    lv_style_t* main_style = (lv_style_t*)lv_malloc(sizeof(lv_style_t));
    lv_style_t* indicator_style = (lv_style_t*)lv_malloc(sizeof(lv_style_t));
    lv_style_t* tick_style = (lv_style_t*)lv_malloc(sizeof(lv_style_t));
    
    // Main style (arc color for round scales, line color for linear)
    lv_style_init(main_style);
    lv_style_set_arc_width(main_style, 8);
    lv_style_set_arc_color(main_style, main_color);
    lv_style_set_line_width(main_style, 8);
    lv_style_set_line_color(main_style, main_color);
    
    // Indicator style (major ticks)
    lv_style_init(indicator_style);
    lv_style_set_line_width(indicator_style, 4);
    lv_style_set_line_color(indicator_style, tick_color);
    
    // Items style (minor ticks)
    lv_style_init(tick_style);
    lv_style_set_line_width(tick_style, 2);
    lv_style_set_line_color(tick_style, tick_color);
    
    lv_scale_section_set_style(section, LV_PART_MAIN, main_style);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, indicator_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, tick_style);
    
    return section;
}

lv_scale_section_t* LVMeter::addProgressSection(lv_color_t color)
{
    if (!m_progressSection) {
        m_progressSection = addSection(m_min, m_value, color, color);
    }
    return m_progressSection;
}

LVMeter* LVMeter::updateSection(lv_scale_section_t* section, int32_t min, int32_t max)
{
    if (section) {
        lv_scale_section_set_range(section, min, max);
        lv_obj_invalidate(m_obj);  // Force redraw
    }
    return this;
}

LVMeter* LVMeter::removeSection(lv_scale_section_t* section)
{
    if (section) {
        // Note: LVGL v9 scale sections are managed internally
        // No explicit delete needed
    }
    return this;
}

LVMeter* LVMeter::enableLineNeedle(uint16_t length, uint16_t width, lv_color_t color)
{
    if (!m_needle) {
        // Create a simple rectangle as needle
        m_needle = lv_obj_create(m_obj);
        m_needleLength = length;
        
        // Remove default styling
        lv_obj_remove_style_all(m_needle);
        
        // Style as thin line
        lv_obj_set_size(m_needle, width, length);
        lv_obj_set_style_bg_color(m_needle, color, 0);
        lv_obj_set_style_bg_opa(m_needle, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(m_needle, width/2, 0);
        
        // Set pivot point at bottom center
        lv_obj_set_style_transform_pivot_x(m_needle, width/2, 0);
        lv_obj_set_style_transform_pivot_y(m_needle, length, 0);
        
        // Position at center
        lv_obj_align(m_needle, LV_ALIGN_CENTER, 0, -length/2);
        
        updateNeedle();
    }
    return this;
}

LVMeter* LVMeter::updateNeedle()
{
    if (!m_needle) return this;
    
    int32_t angle = valueToAngle(m_value);
    
    ESP_LOGI("LVMeter", "updateNeedle: value=%d → angle=%d (%.1f°)", 
             (int)m_value, (int)angle, angle / 10.0f);
    
    // LVGL rotation is in tenths of degrees
    lv_obj_set_style_transform_rotation(m_needle, angle, 0);
    
    return this;
}

LVMeter* LVMeter::enableProgressArc(uint16_t width, lv_color_t color)
{
    if (!m_progressArc) {
        m_progressArc = lv_arc_create(m_obj);
        
        // Match scale's angle range
        lv_obj_set_size(m_progressArc, lv_obj_get_width(m_obj) - 20, lv_obj_get_height(m_obj) - 20);
        lv_obj_center(m_progressArc);
        
        // Remove background, only show indicator
        lv_obj_set_style_arc_opa(m_progressArc, 0, LV_PART_MAIN);
        lv_obj_set_style_arc_width(m_progressArc, width, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(m_progressArc, color, LV_PART_INDICATOR);
        
        // Remove knob
        lv_obj_set_style_opa(m_progressArc, 0, LV_PART_KNOB);
        
        // Set same rotation as scale
        lv_arc_set_rotation(m_progressArc, 135);  // Match default speedometer rotation
        lv_arc_set_bg_angles(m_progressArc, 0, 270);  // Match 270° range
        
        updateProgressArc();
    }
    return this;
}

LVMeter* LVMeter::updateProgressArc()
{
    if (!m_progressArc) return this;
    
    // Map value to arc range (0-100%)
    int32_t range = m_max - m_min;
    int32_t arc_value = (m_value - m_min) * 100 / range;
    
    lv_arc_set_value(m_progressArc, arc_value);
    
    return this;
}

int32_t LVMeter::valueToAngle(int32_t value)
{
    // Map value to angle range
    // Example: rotation=135, angle_range=270
    // value=0 → angle=135° (right side)
    // value=max → angle=135+270=405° = 45° (right side after full sweep)
    int32_t range = m_max - m_min;
    if (range == 0) return m_rotation * 10; // Return rotation in tenths
    
    // Map value to angle offset (0 to angle_range)
    int32_t offset = ((value - m_min) * m_angleRange * 10) / range;
    
    // Negate for mirror correction (-(rotation - offset))
    int32_t angle = -((m_rotation * 10) - offset);
    
    return angle;
}

