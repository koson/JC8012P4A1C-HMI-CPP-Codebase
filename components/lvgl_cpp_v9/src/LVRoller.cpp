/**
 * @file LVRoller.cpp
 * @brief Implementation ของ LVRoller
 */

#include "../include/LVRoller.hpp"
#include <sstream>

LVRoller::LVRoller(LVWidget* parent)
    : LVWidget(parent, lv_roller_create(parent ? parent->obj() : nullptr))
{
    lv_obj_set_user_data(m_obj, this);
}

LVRoller* LVRoller::setOptions(const std::string& options, lv_roller_mode_t mode)
{
    lv_roller_set_options(m_obj, options.c_str(), mode);
    return this;
}

LVRoller* LVRoller::setOptions(const std::vector<std::string>& options, lv_roller_mode_t mode)
{
    std::ostringstream oss;
    for (size_t i = 0; i < options.size(); i++) {
        oss << options[i];
        if (i < options.size() - 1) oss << "\n";
    }
    return setOptions(oss.str(), mode);
}

LVRoller* LVRoller::setSelected(uint16_t index, lv_anim_enable_t anim)
{
    lv_roller_set_selected(m_obj, index, anim);
    return this;
}

uint16_t LVRoller::getSelected() const
{
    return lv_roller_get_selected(m_obj);
}

std::string LVRoller::getSelectedStr() const
{
    char buf[64];
    lv_roller_get_selected_str(m_obj, buf, sizeof(buf));
    return std::string(buf);
}

LVRoller* LVRoller::setVisibleRowCount(uint8_t row_cnt)
{
    lv_roller_set_visible_row_count(m_obj, row_cnt);
    return this;
}

LVRoller* LVRoller::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVRoller::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVRoller* roller = static_cast<LVRoller*>(lv_obj_get_user_data(obj));
    
    if (roller && roller->m_onChange) {
        uint16_t selected = lv_roller_get_selected(obj);
        roller->m_onChange(selected);
    }
}
