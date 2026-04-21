/**
 * @file LVList.cpp
 * @brief Implementation ของ LVList
 */

#include "../include/LVList.hpp"

LVList::LVList(LVWidget* parent)
    : LVWidget(parent, lv_list_create(parent ? parent->obj() : nullptr))
{
}

lv_obj_t* LVList::addText(const char* text)
{
    return lv_list_add_text(m_obj, text);
}

lv_obj_t* LVList::addButton(const char* icon, const char* text)
{
    return lv_list_add_button(m_obj, icon, text);
}

const char* LVList::getButtonText(lv_obj_t* btn) const
{
    return lv_list_get_button_text(m_obj, btn);
}

LVList* LVList::setButtonText(lv_obj_t* btn, const char* text)
{
    lv_list_set_button_text(m_obj, btn, text);
    return this;
}
