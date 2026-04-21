/**
 * @file LVColorWheel.cpp
 * @brief Implementation ของ LVColorWheel (Arc-based color picker)
 */

#include "../include/LVColorWheel.hpp"

LVColorWheel::LVColorWheel(LVWidget* parent, bool knob_recolor)
    : LVWidget(parent, lv_arc_create(parent ? parent->obj() : nullptr)), m_hue(0)
{
    // Create Arc widget as color picker (hue selector)
    
    // Configure as full circle (0-360 degrees = hue)
    lv_arc_set_range(m_obj, 0, 360);
    lv_arc_set_value(m_obj, 0);
    lv_arc_set_bg_angles(m_obj, 0, 360);
    
    // Make it colorful by setting arc color
    lv_obj_set_style_arc_color(m_obj, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
}

LVColorWheel* LVColorWheel::setColor(lv_color_t color)
{
    // Convert RGB to HSV to get hue
    lv_color_hsv_t hsv = lv_color_to_hsv(color);
    m_hue = hsv.h;
    lv_arc_set_value(m_obj, m_hue);
    return this;
}

lv_color_t LVColorWheel::getColor() const
{
    // Convert hue to RGB (full saturation and value)
    lv_color_hsv_t hsv = {m_hue, 100, 100};
    return lv_color_hsv_to_rgb(hsv.h, hsv.s, hsv.v);
}

lv_color_hsv_t LVColorWheel::getColorHSV() const
{
    return {m_hue, 100, 100};
}

LVColorWheel* LVColorWheel::setHue(uint16_t hue)
{
    m_hue = hue % 360;
    lv_arc_set_value(m_obj, m_hue);
    return this;
}

uint16_t LVColorWheel::getHue() const
{
    return m_hue;
}

LVColorWheel* LVColorWheel::onColorChange(OnColorChangeCallback callback)
{
    m_onColorChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVColorWheel::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVColorWheel* wheel = static_cast<LVColorWheel*>(lv_obj_get_user_data(obj));
    
    if (wheel && wheel->m_onColorChange) {
        // Get hue from arc value
        int32_t hue = lv_arc_get_value(obj);
        wheel->m_hue = static_cast<uint16_t>(hue);
        
        // Convert to RGB color
        lv_color_hsv_t hsv = {wheel->m_hue, 100, 100};
        lv_color_t color = lv_color_hsv_to_rgb(hsv.h, hsv.s, hsv.v);
        wheel->m_onColorChange(color);
    }
}
