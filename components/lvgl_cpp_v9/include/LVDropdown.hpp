/**
 * @file LVDropdown.hpp
 * @brief C++ Wrapper สำหรับ LVGL Dropdown Widget
 * 
 * Dropdown = Drop-down list menu
 * คล้าย QComboBox ใน Qt หรือ ComboBox ใน C#
 */

#ifndef LV_DROPDOWN_HPP
#define LV_DROPDOWN_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>
#include <vector>

/**
 * @class LVDropdown
 * @brief Dropdown menu widget
 */
class LVDropdown : public LVWidget {
public:
    using OnChangeCallback = std::function<void(uint16_t selected_index)>;

    explicit LVDropdown(LVWidget* parent = nullptr);

    // Options management
    LVDropdown* setOptions(const std::string& options);  // "Option1\nOption2\nOption3"
    LVDropdown* setOptions(const std::vector<std::string>& options);
    LVDropdown* addOption(const std::string& option, uint32_t pos = 0xFFFF);
    LVDropdown* clearOptions();

    // Selection
    LVDropdown* setSelected(uint16_t index);
    uint16_t getSelected() const;
    std::string getSelectedStr() const;

    // Appearance
    LVDropdown* setDirection(lv_dir_t dir);  // LV_DIR_BOTTOM, LV_DIR_TOP, etc.
    LVDropdown* setSymbol(const char* symbol);

    // Events
    LVDropdown* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_DROPDOWN_HPP
