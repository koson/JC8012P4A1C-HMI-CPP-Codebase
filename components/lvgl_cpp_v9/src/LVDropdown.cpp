/**
 * @file LVDropdown.cpp
 * @brief Implementation ของ LVDropdown
 */

#include "../include/LVDropdown.hpp"
#include <sstream>

LVDropdown::LVDropdown(LVWidget* parent)
    : LVWidget(parent, lv_dropdown_create(parent ? parent->obj() : lv_scr_act()))
{
    // m_obj already set by base class
}

LVDropdown* LVDropdown::setOptions(const std::string& options)
{
    lv_dropdown_set_options(m_obj, options.c_str());
    return this;
}

LVDropdown* LVDropdown::setOptions(const std::vector<std::string>& options)
{
    std::ostringstream oss;
    for (size_t i = 0; i < options.size(); i++) {
        oss << options[i];
        if (i < options.size() - 1) oss << "\n";
    }
    return setOptions(oss.str());
}

LVDropdown* LVDropdown::addOption(const std::string& option, uint32_t pos)
{
    lv_dropdown_add_option(m_obj, option.c_str(), pos);
    return this;
}

LVDropdown* LVDropdown::clearOptions()
{
    lv_dropdown_clear_options(m_obj);
    return this;
}

LVDropdown* LVDropdown::setSelected(uint16_t index)
{
    lv_dropdown_set_selected(m_obj, index);
    return this;
}

uint16_t LVDropdown::getSelected() const
{
    return lv_dropdown_get_selected(m_obj);
}

std::string LVDropdown::getSelectedStr() const
{
    char buf[64];
    lv_dropdown_get_selected_str(m_obj, buf, sizeof(buf));
    return std::string(buf);
}

LVDropdown* LVDropdown::setDirection(lv_dir_t dir)
{
    lv_dropdown_set_dir(m_obj, dir);
    return this;
}

LVDropdown* LVDropdown::setSymbol(const char* symbol)
{
    lv_dropdown_set_symbol(m_obj, symbol);
    return this;
}

LVDropdown* LVDropdown::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVDropdown::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVDropdown* dd = static_cast<LVDropdown*>(lv_obj_get_user_data(obj));
    
    if (dd && dd->m_onChange) {
        uint16_t selected = lv_dropdown_get_selected(obj);
        dd->m_onChange(selected);
    }
}
