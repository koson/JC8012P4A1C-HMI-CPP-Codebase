/**
 * @file LVSpinbox.cpp
 * @brief Implementation ของ LVSpinbox
 */

#include "../include/LVSpinbox.hpp"

LVSpinbox::LVSpinbox(LVWidget* parent)
    : LVWidget(parent, lv_spinbox_create(parent ? parent->obj() : nullptr))
{
}

LVSpinbox* LVSpinbox::setValue(int32_t value)
{
    lv_spinbox_set_value(m_obj, value);
    return this;
}

int32_t LVSpinbox::getValue() const
{
    return lv_spinbox_get_value(m_obj);
}

LVSpinbox* LVSpinbox::increment()
{
    lv_spinbox_increment(m_obj);
    return this;
}

LVSpinbox* LVSpinbox::decrement()
{
    lv_spinbox_decrement(m_obj);
    return this;
}

LVSpinbox* LVSpinbox::setRange(int32_t min, int32_t max)
{
    lv_spinbox_set_range(m_obj, min, max);
    return this;
}

LVSpinbox* LVSpinbox::setDigitFormat(uint8_t digit_count, uint8_t separator_position)
{
    lv_spinbox_set_digit_format(m_obj, digit_count, separator_position);
    return this;
}

LVSpinbox* LVSpinbox::setStep(uint32_t step)
{
    lv_spinbox_set_step(m_obj, step);
    return this;
}

LVSpinbox* LVSpinbox::setRollover(bool enable)
{
    lv_spinbox_set_rollover(m_obj, enable);
    return this;
}

LVSpinbox* LVSpinbox::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVSpinbox::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVSpinbox* spinbox = static_cast<LVSpinbox*>(lv_obj_get_user_data(obj));
    
    if (spinbox && spinbox->m_onChange) {
        int32_t value = lv_spinbox_get_value(obj);
        spinbox->m_onChange(value);
    }
}
