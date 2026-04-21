/**
 * @file LVTabView.hpp
 * @brief C++ Wrapper สำหรับ LVGL TabView Widget
 * 
 * TabView = Tab container with multiple pages
 * คล้าย QTabWidget ใน Qt หรือ TabControl ใน C#
 */

#ifndef LV_TABVIEW_HPP
#define LV_TABVIEW_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>

/**
 * @class LVTabView
 * @brief Tab view widget สำหรับแสดงหลายหน้าด้วย tabs
 */
class LVTabView : public LVWidget {
public:
    using OnTabChangeCallback = std::function<void(uint32_t tab_index)>;

    explicit LVTabView(LVWidget* parent = nullptr, lv_dir_t tab_pos = LV_DIR_TOP);

    // Tab management
    lv_obj_t* addTab(const char* name);
    LVTabView* renameTab(uint32_t index, const char* new_name);
    
    // Tab navigation
    LVTabView* setActiveTab(uint32_t index, lv_anim_enable_t anim = LV_ANIM_OFF);
    uint32_t getActiveTab() const;
    lv_obj_t* getTabContent(uint32_t index) const;
    
    // Tab bar control
    LVTabView* setTabBarPosition(lv_dir_t pos);
    LVTabView* setTabBarSize(int32_t size);
    
    // Events
    LVTabView* onTabChange(OnTabChangeCallback callback);

private:
    OnTabChangeCallback m_onTabChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_TABVIEW_HPP
