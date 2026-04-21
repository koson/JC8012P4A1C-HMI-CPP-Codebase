/**
 * @file LVMenu.hpp
 * @brief C++ Wrapper สำหรับ LVGL Menu Widget
 * 
 * Menu = Nested menu structure
 * คล้าย QMenu ใน Qt หรือ MenuStrip ใน C#
 */

#ifndef LV_MENU_HPP
#define LV_MENU_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVMenu
 * @brief Menu widget สำหรับสร้างเมนูแบบ nested
 */
class LVMenu : public LVWidget {
public:
    explicit LVMenu(LVWidget* parent = nullptr);

    // Page management
    lv_obj_t* createPage(const char* title);
    lv_obj_t* createSubPage(lv_obj_t* parent_page, const char* title);
    
    // Item creation
    lv_obj_t* createSection(lv_obj_t* page, const char* title);
    lv_obj_t* createContainerItem(lv_obj_t* page, const char* icon, const char* text, lv_obj_t* sub_page);
    lv_obj_t* createSwitchItem(lv_obj_t* page, const char* icon, const char* text);
    lv_obj_t* createSliderItem(lv_obj_t* page, const char* icon, const char* text, int32_t min, int32_t max, int32_t value);
    
    // Navigation
    LVMenu* setPage(lv_obj_t* page);
    LVMenu* setMainPage(lv_obj_t* page);
    LVMenu* clearHistory();
    lv_obj_t* getBackButton() const;
    
    // Header
    lv_obj_t* getHeader() const;
    lv_obj_t* getMainHeaderContainer() const;

private:
    lv_obj_t* m_header;
};

#endif // LV_MENU_HPP
