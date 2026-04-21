/**
 * @file LVSwitch.hpp
 * @brief C++ Wrapper สำหรับ LVGL Switch Widget
 * 
 * Switch = สวิตช์เปิด/ปิด (คล้าย toggle switch ใน iOS/Android)
 */

#ifndef LV_SWITCH_HPP
#define LV_SWITCH_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVSwitch
 * @brief สวิตช์เปิด/ปิด
 */
class LVSwitch : public LVWidget {
public:
    using OnChangeCallback = std::function<void(bool on)>;

    explicit LVSwitch(LVWidget* parent = nullptr);

    /**
     * @brief เปิด/ปิด switch
     * @param on true = เปิด, false = ปิด
     * @param anim มี animation หรือไม่
     * @return this*
     */
    LVSwitch* setOn(bool on, bool anim = false);

    /**
     * @brief เช็คว่าเปิดอยู่หรือไม่
     * @return true = เปิด, false = ปิด
     */
    bool isOn() const;

    /**
     * @brief Toggle สถานะ
     * @return this*
     */
    LVSwitch* toggle();

    /**
     * @brief Callback เมื่อสถานะเปลี่ยน
     */
    LVSwitch* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_SWITCH_HPP
