/**
 * @file LVRoller.hpp
 * @brief C++ Wrapper สำหรับ LVGL Roller Widget
 * 
 * Roller = Rolling selector wheel
 * คล้าย Wheel Picker ใน iOS หรือ NumberPicker ใน Android
 */

#ifndef LV_ROLLER_HPP
#define LV_ROLLER_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>
#include <vector>

/**
 * @class LVRoller
 * @brief Rolling selector widget
 */
class LVRoller : public LVWidget {
public:
    using OnChangeCallback = std::function<void(uint16_t selected_index)>;

    explicit LVRoller(LVWidget* parent = nullptr);

    // Options management
    LVRoller* setOptions(const std::string& options, lv_roller_mode_t mode = LV_ROLLER_MODE_NORMAL);
    LVRoller* setOptions(const std::vector<std::string>& options, lv_roller_mode_t mode = LV_ROLLER_MODE_NORMAL);

    // Selection
    LVRoller* setSelected(uint16_t index, lv_anim_enable_t anim = LV_ANIM_OFF);
    uint16_t getSelected() const;
    std::string getSelectedStr() const;

    // Appearance
    LVRoller* setVisibleRowCount(uint8_t row_cnt);

    // Events
    LVRoller* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_ROLLER_HPP
