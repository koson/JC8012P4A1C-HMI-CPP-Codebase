/**
 * @file LVList.hpp
 * @brief C++ Wrapper สำหรับ LVGL List Widget
 * 
 * List = Scrollable list with text and button items
 * คล้าย ListView ใน Android
 */

#ifndef LV_LIST_HPP
#define LV_LIST_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>

/**
 * @class LVList
 * @brief List widget สำหรับแสดงรายการ
 */
class LVList : public LVWidget {
public:
    explicit LVList(LVWidget* parent = nullptr);

    // Item management
    lv_obj_t* addText(const char* text);
    lv_obj_t* addButton(const char* icon, const char* text);
    
    // Utility
    const char* getButtonText(lv_obj_t* btn) const;
    LVList* setButtonText(lv_obj_t* btn, const char* text);
};

#endif // LV_LIST_HPP
