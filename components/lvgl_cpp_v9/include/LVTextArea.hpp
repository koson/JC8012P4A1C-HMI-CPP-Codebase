/**
 * @file LVTextArea.hpp
 * @brief C++ Wrapper สำหรับ LVGL TextArea Widget
 * 
 * TextArea = Multi-line text input field
 * คล้าย QTextEdit ใน Qt หรือ TextBox (Multiline) ใน C#
 */

#ifndef LV_TEXTAREA_HPP
#define LV_TEXTAREA_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>

/**
 * @class LVTextArea
 * @brief Multi-line text input field
 */
class LVTextArea : public LVWidget {
public:
    using OnChangeCallback = std::function<void(const std::string& text)>;

    explicit LVTextArea(LVWidget* parent = nullptr);

    // Text operations
    LVTextArea* setText(const std::string& text);
    std::string getText() const;
    LVTextArea* addText(const std::string& text);  // Append text
    LVTextArea* clearText();

    // Placeholder
    LVTextArea* setPlaceholder(const std::string& placeholder);

    // Input control
    LVTextArea* setMaxLength(uint32_t max_len);
    LVTextArea* setOneLineMode(bool enable);       // Single line mode
    LVTextArea* setPasswordMode(bool enable);      // Hide characters

    // Cursor
    LVTextArea* setCursorPos(uint32_t pos);
    uint32_t getCursorPos() const;
    LVTextArea* setCursorClickPos(bool enable);    // Click to move cursor

    // Selection
    LVTextArea* setTextSelection(bool enable);
    bool textIsSelected() const;

    // Accepted characters
    LVTextArea* setAcceptedChars(const char* list);  // e.g., "0123456789"

    // Events
    LVTextArea* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_TEXTAREA_HPP
