/**
 * @file LVSpinner.cpp
 * @brief Implementation ของ LVSpinner
 */

#include "../include/LVSpinner.hpp"

LVSpinner::LVSpinner(LVWidget* parent)
    : LVWidget(parent, lv_spinner_create(parent ? parent->obj() : nullptr))
{
}

LVSpinner* LVSpinner::setSpeed(uint32_t speed_ms)
{
    lv_spinner_set_anim_params(m_obj, speed_ms, 60);  // speed, angle
    return this;
}

LVSpinner* LVSpinner::setArcLength(uint16_t arc_length)
{
    lv_spinner_set_anim_params(m_obj, 1000, arc_length);
    return this;
}
