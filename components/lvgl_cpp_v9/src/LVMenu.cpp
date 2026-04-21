/**
 * @file LVMenu.cpp
 * @brief Implementation ของ LVMenu
 */

#include "../include/LVMenu.hpp"

LVMenu::LVMenu(LVWidget* parent)
    : LVWidget(parent, lv_menu_create(parent ? parent->obj() : nullptr)), m_header(nullptr)
{
}

lv_obj_t* LVMenu::createPage(const char* title)
{
    return lv_menu_page_create(m_obj, title);
}

lv_obj_t* LVMenu::createSubPage(lv_obj_t* parent_page, const char* title)
{
    return lv_menu_page_create(m_obj, title);
}

lv_obj_t* LVMenu::createSection(lv_obj_t* page, const char* title)
{
    return lv_menu_section_create(page);
}

lv_obj_t* LVMenu::createContainerItem(lv_obj_t* page, const char* icon, const char* text, lv_obj_t* sub_page)
{
    return lv_menu_cont_create(page);
}

lv_obj_t* LVMenu::createSwitchItem(lv_obj_t* page, const char* icon, const char* text)
{
    return lv_menu_cont_create(page);
}

lv_obj_t* LVMenu::createSliderItem(lv_obj_t* page, const char* icon, const char* text, int32_t min, int32_t max, int32_t value)
{
    return lv_menu_cont_create(page);
}

LVMenu* LVMenu::setPage(lv_obj_t* page)
{
    lv_menu_set_page(m_obj, page);
    return this;
}

LVMenu* LVMenu::setMainPage(lv_obj_t* page)
{
    lv_menu_set_page(m_obj, page);
    return this;
}

LVMenu* LVMenu::clearHistory()
{
    lv_menu_clear_history(m_obj);
    return this;
}

lv_obj_t* LVMenu::getBackButton() const
{
    return lv_menu_get_main_header_back_button(m_obj);
}

lv_obj_t* LVMenu::getHeader() const
{
    return lv_menu_get_main_header(m_obj);
}

lv_obj_t* LVMenu::getMainHeaderContainer() const
{
    return lv_menu_get_main_header_back_button(m_obj);
}
