/**
 * @file LVCheckbox.cpp
 * @brief Implementation ของ LVCheckbox
 */

#include "../include/LVCheckbox.hpp"

LVCheckbox::LVCheckbox(LVWidget* parent, const std::string& text)
    : LVWidget(parent, lv_checkbox_create(parent ? parent->obj() : lv_scr_act()))
{
    if (!m_obj) {
        // Failed to create checkbox object
        return;
    }
    
    // ตั้งข้อความถ้ามี
    if (!text.empty()) {
        lv_checkbox_set_text(m_obj, text.c_str());
    }
}

LVCheckbox* LVCheckbox::setText(const std::string& text)
{
    lv_checkbox_set_text(m_obj, text.c_str());
    return this;
}

LVCheckbox* LVCheckbox::setChecked(bool checked)
{
    // LVGL ใช้ state flags
    if (checked) {
        lv_obj_add_state(m_obj, LV_STATE_CHECKED);
    } else {
        lv_obj_remove_state(m_obj, LV_STATE_CHECKED);
    }
    return this;
}

bool LVCheckbox::isChecked() const
{
    // เช็คว่ามี state CHECKED หรือไม่
    return lv_obj_has_state(m_obj, LV_STATE_CHECKED);
}

LVCheckbox* LVCheckbox::toggle()
{
    // Toggle = เปลี่ยนสถานะ
    setChecked(!isChecked());
    return this;
}

LVCheckbox* LVCheckbox::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVCheckbox::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVCheckbox* checkbox = static_cast<LVCheckbox*>(lv_obj_get_user_data(obj));
    
    if (checkbox && checkbox->m_onChange) {
        bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
        checkbox->m_onChange(checked);
    }
}
