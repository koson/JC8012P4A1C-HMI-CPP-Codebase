/**
 * @file LVColorWheel.hpp
 * @brief C++ Wrapper สำหรับ Color Picker (using Arc widget)
 * 
 * Note: LVGL v9 removed lv_colorwheel, using Arc as alternative
 * Simple color picker using Arc for hue selection
 */

#ifndef LV_COLORWHEEL_HPP
#define LV_COLORWHEEL_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVColorWheel
 * @brief Simple color picker widget (Arc-based)
 */
class LVColorWheel : public LVWidget {
public:
    using OnColorChangeCallback = std::function<void(lv_color_t color)>;

    explicit LVColorWheel(LVWidget* parent = nullptr, bool knob_recolor = true);

    // Color operations
    LVColorWheel* setColor(lv_color_t color);
    lv_color_t getColor() const;
    lv_color_hsv_t getColorHSV() const;
    
    // Hue control (0-360)
    LVColorWheel* setHue(uint16_t hue);
    uint16_t getHue() const;
    
    // Events
    LVColorWheel* onColorChange(OnColorChangeCallback callback);

private:
    OnColorChangeCallback m_onColorChange;
    uint16_t m_hue = 0;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_COLORWHEEL_HPP
