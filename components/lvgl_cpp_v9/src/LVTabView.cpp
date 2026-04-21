/**
 * @file LVTabView.cpp
 * @brief Implementation ของ LVTabView
 */

#include "../include/LVTabView.hpp"

LVTabView::LVTabView(LVWidget* parent, lv_dir_t tab_pos)
    : LVWidget(parent, lv_tabview_create(parent ? parent->obj() : nullptr))
{
    lv_tabview_set_tab_bar_position(m_obj, tab_pos);
}

lv_obj_t* LVTabView::addTab(const char* name)
{
    return lv_tabview_add_tab(m_obj, name);
}

LVTabView* LVTabView::renameTab(uint32_t index, const char* new_name)
{
    lv_tabview_rename_tab(m_obj, index, new_name);
    return this;
}

LVTabView* LVTabView::setActiveTab(uint32_t index, lv_anim_enable_t anim)
{
    lv_tabview_set_active(m_obj, index, anim);
    return this;
}

uint32_t LVTabView::getActiveTab() const
{
    return lv_tabview_get_tab_active(m_obj);
}

lv_obj_t* LVTabView::getTabContent(uint32_t index) const
{
    return lv_tabview_get_tab_bar(m_obj);
}

LVTabView* LVTabView::setTabBarPosition(lv_dir_t pos)
{
    lv_tabview_set_tab_bar_position(m_obj, pos);
    return this;
}

LVTabView* LVTabView::setTabBarSize(int32_t size)
{
    lv_tabview_set_tab_bar_size(m_obj, size);
    return this;
}

LVTabView* LVTabView::onTabChange(OnTabChangeCallback callback)
{
    m_onTabChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVTabView::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVTabView* tabview = static_cast<LVTabView*>(lv_obj_get_user_data(obj));
    
    if (tabview && tabview->m_onTabChange) {
        uint32_t active = lv_tabview_get_tab_active(obj);
        tabview->m_onTabChange(active);
    }
}
