/**
 * @file LVTextArea.cpp
 * @brief Implementation ของ LVTextArea
 */

#include "../include/LVTextArea.hpp"

LVTextArea::LVTextArea(LVWidget* parent)
    : LVWidget(parent, lv_textarea_create(parent ? parent->obj() : nullptr))
{
    lv_obj_set_user_data(m_obj, this);
}

LVTextArea* LVTextArea::setText(const std::string& text)
{
    lv_textarea_set_text(m_obj, text.c_str());
    return this;
}

std::string LVTextArea::getText() const
{
    const char* text = lv_textarea_get_text(m_obj);
    return text ? std::string(text) : "";
}

LVTextArea* LVTextArea::addText(const std::string& text)
{
    lv_textarea_add_text(m_obj, text.c_str());
    return this;
}

LVTextArea* LVTextArea::clearText()
{
    lv_textarea_set_text(m_obj, "");
    return this;
}

LVTextArea* LVTextArea::setPlaceholder(const std::string& placeholder)
{
    lv_textarea_set_placeholder_text(m_obj, placeholder.c_str());
    return this;
}

LVTextArea* LVTextArea::setMaxLength(uint32_t max_len)
{
    lv_textarea_set_max_length(m_obj, max_len);
    return this;
}

LVTextArea* LVTextArea::setOneLineMode(bool enable)
{
    lv_textarea_set_one_line(m_obj, enable);
    return this;
}

LVTextArea* LVTextArea::setPasswordMode(bool enable)
{
    lv_textarea_set_password_mode(m_obj, enable);
    return this;
}

LVTextArea* LVTextArea::setCursorPos(uint32_t pos)
{
    lv_textarea_set_cursor_pos(m_obj, pos);
    return this;
}

uint32_t LVTextArea::getCursorPos() const
{
    return lv_textarea_get_cursor_pos(m_obj);
}

LVTextArea* LVTextArea::setCursorClickPos(bool enable)
{
    lv_textarea_set_cursor_click_pos(m_obj, enable);
    return this;
}

LVTextArea* LVTextArea::setTextSelection(bool enable)
{
    lv_textarea_set_text_selection(m_obj, enable);
    return this;
}

bool LVTextArea::textIsSelected() const
{
    return lv_textarea_text_is_selected(m_obj);
}

LVTextArea* LVTextArea::setAcceptedChars(const char* list)
{
    lv_textarea_set_accepted_chars(m_obj, list);
    return this;
}

LVTextArea* LVTextArea::onChange(OnChangeCallback callback)
{
    m_onChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVTextArea::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVTextArea* ta = static_cast<LVTextArea*>(lv_obj_get_user_data(obj));
    
    if (ta && ta->m_onChange) {
        std::string text = ta->getText();
        ta->m_onChange(text);
    }
}
