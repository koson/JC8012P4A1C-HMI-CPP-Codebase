/**
 * @file LVSpinbox.hpp
 * @brief C++ Wrapper สำหรับ LVGL Spinbox Widget
 * 
 * Spinbox = Numeric input with increment/decrement buttons
 * คล้าย QSpinBox ใน Qt หรือ NumericUpDown ใน C#
 */

#ifndef LV_SPINBOX_HPP
#define LV_SPINBOX_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVSpinbox
 * @brief Spinbox widget สำหรับป้อนตัวเลข
 */
class LVSpinbox : public LVWidget {
public:
    using OnChangeCallback = std::function<void(int32_t value)>;

    explicit LVSpinbox(LVWidget* parent = nullptr);

    // Value operations
    LVSpinbox* setValue(int32_t value);
    int32_t getValue() const;
    LVSpinbox* increment();
    LVSpinbox* decrement();

    // Range and format
    LVSpinbox* setRange(int32_t min, int32_t max);
    LVSpinbox* setDigitFormat(uint8_t digit_count, uint8_t separator_position);
    LVSpinbox* setStep(uint32_t step);

    // Rollover behavior
    LVSpinbox* setRollover(bool enable);

    // Events
    LVSpinbox* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_SPINBOX_HPP
