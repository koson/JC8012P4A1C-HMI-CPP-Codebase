/**
 * @file LVWindow.hpp
 * @brief C++ Wrapper สำหรับ LVGL Window Widget
 * 
 * Window = Panel with header and optional close button
 * คล้าย QDialog ใน Qt หรือ Form ใน C#
 */

#ifndef LV_WINDOW_HPP
#define LV_WINDOW_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>

/**
 * @class LVWindow
 * @brief Window widget พร้อม header และปุ่มปิด
 */
class LVWindow : public LVWidget {
public:
    using OnCloseCallback = std::function<void()>;

    explicit LVWindow(LVWidget* parent = nullptr, int32_t header_height = 40);

    // Header management
    lv_obj_t* getHeader() const;
    lv_obj_t* getContent() const;
    LVWindow* addHeaderButton(const char* icon, int32_t width);
    LVWindow* setTitle(const char* title);
    
    // Events
    LVWindow* onClose(OnCloseCallback callback);

private:
    lv_obj_t* m_header;
    lv_obj_t* m_content;
    OnCloseCallback m_onClose;
};

#endif // LV_WINDOW_HPP
