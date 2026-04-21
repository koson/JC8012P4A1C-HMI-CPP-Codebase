/**
 * @file LVBar.cpp
 * @brief Implementation ของ LVBar
 */

#include "../include/LVBar.hpp"

LVBar::LVBar(LVWidget* parent)
    : LVWidget(parent, lv_bar_create(parent ? parent->obj() : lv_scr_act()))
{
    if (!m_obj) return;
    
    lv_bar_set_range(m_obj, 0, 100);  // ค่าเริ่มต้น 0-100
}

LVBar* LVBar::setRange(int32_t min, int32_t max)
{
    lv_bar_set_range(m_obj, min, max);
    return this;
}

LVBar* LVBar::setValue(int32_t value, bool anim)
{
    lv_bar_set_value(m_obj, value, anim ? LV_ANIM_ON : LV_ANIM_OFF);
    return this;
}

int32_t LVBar::getValue() const
{
    return lv_bar_get_value(m_obj);
}

LVBar* LVBar::setStartValue(int32_t value, bool anim)
{
    lv_bar_set_start_value(m_obj, value, anim ? LV_ANIM_ON : LV_ANIM_OFF);
    return this;
}

int32_t LVBar::getStartValue() const
{
    return lv_bar_get_start_value(m_obj);
}

LVBar* LVBar::setMode(lv_bar_mode_t mode)
{
    lv_bar_set_mode(m_obj, mode);
    return this;
}
