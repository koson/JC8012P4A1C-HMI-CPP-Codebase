/**
 * @file LVWindow.cpp
 * @brief Implementation ของ LVWindow
 */

#include "../include/LVWindow.hpp"

LVWindow::LVWindow(LVWidget* parent, int32_t header_height)
    : LVWidget(parent, lv_win_create(parent ? parent->obj() : nullptr)), m_header(nullptr), m_content(nullptr)
{
    
    // Get header and content parts
    m_header = lv_win_get_header(m_obj);
    m_content = lv_win_get_content(m_obj);
}

lv_obj_t* LVWindow::getHeader() const
{
    return m_header;
}

lv_obj_t* LVWindow::getContent() const
{
    return m_content;
}

LVWindow* LVWindow::addHeaderButton(const char* icon, int32_t width)
{
    lv_win_add_button(m_obj, icon, width);
    return this;
}

LVWindow* LVWindow::setTitle(const char* title)
{
    lv_win_add_title(m_obj, title);
    return this;
}

LVWindow* LVWindow::onClose(OnCloseCallback callback)
{
    m_onClose = callback;
    return this;
}
