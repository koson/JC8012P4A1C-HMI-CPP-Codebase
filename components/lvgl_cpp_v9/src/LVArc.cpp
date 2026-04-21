/**
 * @file LVArc.cpp
 * @brief Implementation ของ LVArc
 */

#include "../include/LVArc.hpp"

LVArc::LVArc(LVWidget* parent)
    : LVWidget(parent, lv_arc_create(parent ? parent->obj() : lv_scr_act()))
{
    if (!m_obj) return;
    
    lv_arc_set_range(m_obj, 0, 100);
}

LVArc* LVArc::setRange(int32_t min, int32_t max)
{
    lv_arc_set_range(m_obj, min, max);
    return this;
}

LVArc* LVArc::setValue(int32_t value)
{
    lv_arc_set_value(m_obj, value);
    return this;
}

int32_t LVArc::getValue() const
{
    return lv_arc_get_value(m_obj);
}

LVArc* LVArc::setAngles(int32_t start, int32_t end)
{
    lv_arc_set_bg_angles(m_obj, start, end);
    return this;
}

LVArc* LVArc::setRotation(int32_t rotation)
{
    lv_arc_set_rotation(m_obj, rotation);
    return this;
}

LVArc* LVArc::setMode(lv_arc_mode_t mode)
{
    lv_arc_set_mode(m_obj, mode);
    return this;
}

LVArc* LVArc::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVArc::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVArc* arc = static_cast<LVArc*>(lv_obj_get_user_data(obj));
    
    if (arc && arc->m_onChange) {
        int32_t value = lv_arc_get_value(obj);
        arc->m_onChange(value);
    }
}
