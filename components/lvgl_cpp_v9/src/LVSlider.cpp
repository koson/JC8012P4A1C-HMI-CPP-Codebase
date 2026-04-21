/**
 * @file LVSlider.cpp
 * @brief Implementation ของ LVSlider Widget
 * 
 * ไฟล์นี้แสดง PATTERN การ implement widget wrapper:
 * - Constructor: สร้าง LVGL object
 * - Methods: ห่อหุ้ม LVGL C functions
 * - Event handling: เชื่อม LVGL events กับ C++ callbacks
 */

#include "../include/LVSlider.hpp"

/**
 * Constructor Pattern (FIXED):
 * =============================
 * 1. สร้าง slider object ก่อน
 * 2. เรียก parent constructor ส่ง (parent, obj) เพื่อ initialize
 * 3. ไม่ต้อง override m_obj (parent จัดการให้แล้ว)
 */
LVSlider::LVSlider(LVWidget* parent)
    : LVWidget(parent, lv_slider_create(parent ? parent->obj() : lv_scr_act()))
{
    // ตั้งค่าเริ่มต้น
    lv_slider_set_range(m_obj, 0, 100);  // ค่าเริ่มต้น 0-100
    
    // user_data ถูกตั้งค่าใน LVWidget::initialize() แล้ว
}

/**
 * Method Pattern:
 * ===============
 * 1. ห่อหุ้ม LVGL C function
 * 2. Return this* เพื่อทำ method chaining
 * 3. ใช้ชื่อที่เข้าใจง่ายกว่า C version
 */
LVSlider* LVSlider::setRange(int32_t min, int32_t max)
{
    // เรียก LVGL C function โดยตรง
    lv_slider_set_range(m_obj, min, max);
    return this;  // Return this* สำหรับ chaining
}

LVSlider* LVSlider::setValue(int32_t value, bool anim)
{
    // LVGL ใช้ LV_ANIM_ON/OFF enum แต่เราใช้ bool ให้ง่าย
    lv_slider_set_value(m_obj, value, anim ? LV_ANIM_ON : LV_ANIM_OFF);
    return this;
}

int32_t LVSlider::getValue() const
{
    // Getter ไม่ต้อง return this* (ไม่ได้ chain)
    return lv_slider_get_value(m_obj);
}

LVSlider* LVSlider::setLeftValue(int32_t value, bool anim)
{
    // สำหรับ range slider (มี 2 knobs)
    lv_slider_set_left_value(m_obj, value, anim ? LV_ANIM_ON : LV_ANIM_OFF);
    return this;
}

int32_t LVSlider::getLeftValue() const
{
    return lv_slider_get_left_value(m_obj);
}

LVSlider* LVSlider::setMode(bool isRange)
{
    // LV_SLIDER_MODE_NORMAL = slider ธรรมดา (1 knob)
    // LV_SLIDER_MODE_RANGE = range slider (2 knobs)
    lv_slider_set_mode(m_obj, isRange ? LV_SLIDER_MODE_RANGE : LV_SLIDER_MODE_NORMAL);
    return this;
}

LVSlider* LVSlider::setSymmetric(bool enable)
{
    // Symmetric mode ทำให้ slider มีจุดกลางที่ 0
    lv_slider_set_mode(m_obj, enable ? LV_SLIDER_MODE_SYMMETRICAL : LV_SLIDER_MODE_NORMAL);
    return this;
}

/**
 * Event Callback Pattern:
 * =======================
 * 1. เก็บ std::function ใน member variable
 * 2. Add LVGL event listener ชื่อ eventHandler (static)
 * 3. eventHandler ดึง this* จาก user_data
 * 4. เรียก std::function ที่เก็บไว้
 */
LVSlider* LVSlider::onChange(OnChangeCallback callback)
{
    // เก็บ callback ไว้
    m_onChange = callback;
    
    // Register event handler กับ LVGL
    // LV_EVENT_VALUE_CHANGED = เมื่อค่าเปลี่ยน
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    
    return this;
}

/**
 * Static Event Handler Pattern:
 * ==============================
 * ฟังก์ชันนี้เป็น bridge ระหว่าง LVGL C events และ C++ callbacks
 */
void LVSlider::eventHandler(lv_event_t* e)
{
    // ดึง target object (slider) จาก event
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    
    // ดึง this pointer จาก user_data (ที่เก็บไว้ตอน construct)
    LVSlider* slider = static_cast<LVSlider*>(lv_obj_get_user_data(obj));
    
    // เช็คว่ามี callback และ slider instance ถูกต้อง
    if (slider && slider->m_onChange) {
        // อ่านค่าปัจจุบันและเรียก callback
        int32_t value = lv_slider_get_value(obj);
        slider->m_onChange(value);
    }
}
