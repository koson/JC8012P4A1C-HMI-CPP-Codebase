/**
 * @file LVSwitch.cpp
 * @brief Implementation ของ LVSwitch
 */

#include "../include/LVSwitch.hpp"

LVSwitch::LVSwitch(LVWidget* parent)
    : LVWidget(parent, lv_switch_create(parent ? parent->obj() : lv_scr_act()))
{
}

LVSwitch* LVSwitch::setOn(bool on, bool anim)
{
    if (on) {
        lv_obj_add_state(m_obj, LV_STATE_CHECKED);
    } else {
        lv_obj_remove_state(m_obj, LV_STATE_CHECKED);
    }
    return this;
}

bool LVSwitch::isOn() const
{
    return lv_obj_has_state(m_obj, LV_STATE_CHECKED);
}

LVSwitch* LVSwitch::toggle()
{
    setOn(!isOn());
    return this;
}

LVSwitch* LVSwitch::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVSwitch::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVSwitch* sw = static_cast<LVSwitch*>(lv_obj_get_user_data(obj));
    
    if (sw && sw->m_onChange) {
        bool on = lv_obj_has_state(obj, LV_STATE_CHECKED);
        sw->m_onChange(on);
    }
}
