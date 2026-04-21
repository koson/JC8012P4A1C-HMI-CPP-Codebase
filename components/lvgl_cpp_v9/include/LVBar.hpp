/**
 * @file LVBar.hpp
 * @brief C++ Wrapper สำหรับ LVGL Bar Widget
 * 
 * Bar = แถบแสดงความคืบหน้า (Progress Bar)
 * คล้าย CProgressCtrl ใน MFC หรือ <progress> ใน HTML
 */

#ifndef LV_BAR_HPP
#define LV_BAR_HPP

#include "LVWidget.hpp"

/**
 * @class LVBar
 * @brief แถบแสดงความคืบหน้า (Progress Bar)
 */
class LVBar : public LVWidget {
public:
    explicit LVBar(LVWidget* parent = nullptr);

    /**
     * @brief กำหนดช่วงค่า
     * @param min ค่าต่ำสุด
     * @param max ค่าสูงสุด
     * @return this*
     */
    LVBar* setRange(int32_t min, int32_t max);

    /**
     * @brief กำหนดค่าปัจจุบัน
     * @param value ค่า
     * @param anim มี animation หรือไม่
     * @return this*
     */
    LVBar* setValue(int32_t value, bool anim = false);

    /**
     * @brief อ่านค่าปัจจุบัน
     */
    int32_t getValue() const;

    /**
     * @brief กำหนดค่าเริ่มต้น (สำหรับ range bar)
     * @param value ค่าเริ่มต้น
     * @param anim มี animation หรือไม่
     * @return this*
     */
    LVBar* setStartValue(int32_t value, bool anim = false);

    /**
     * @brief อ่านค่าเริ่มต้น
     */
    int32_t getStartValue() const;

    /**
     * @brief กำหนดโหมด (normal, symmetrical, range)
     * @param mode โหมด
     * @return this*
     */
    LVBar* setMode(lv_bar_mode_t mode);
};

#endif // LV_BAR_HPP
