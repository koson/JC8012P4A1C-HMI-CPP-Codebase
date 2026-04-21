/**
 * @file LVSpinner.hpp
 * @brief C++ Wrapper สำหรับ LVGL Spinner Widget
 * 
 * Spinner = Loading indicator / Activity indicator
 * คล้าย ProgressBar แบบหมุนวน (circular loading)
 */

#ifndef LV_SPINNER_HPP
#define LV_SPINNER_HPP

#include "LVWidget.hpp"

/**
 * @class LVSpinner
 * @brief Loading spinner widget
 */
class LVSpinner : public LVWidget {
public:
    explicit LVSpinner(LVWidget* parent = nullptr);

    // Animation control
    LVSpinner* setSpeed(uint32_t speed_ms);  // Time for one revolution
    
    // Arc appearance
    LVSpinner* setArcLength(uint16_t arc_length);  // 0-360 degrees
};

#endif // LV_SPINNER_HPP
