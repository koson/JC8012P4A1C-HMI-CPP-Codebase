#ifndef LVGL_BUTTON_H
#define LVGL_BUTTON_H

#include "lvgl_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Button Widget Class - คล้าย CButton ใน MFC
 * 
 * Derived class จาก LVWidget สำหรับ Button
 * รองรับการแสดงข้อความ, ไอคอน, และ event handling
 */

typedef struct LVButton LVButton;

/**
 * @brief Button Structure - inherit จาก LVWidget
 */
struct LVButton {
    LVWidget base;          // Base class (ต้องอยู่ตำแหน่งแรก - สำคัญมากสำหรับ inheritance ใน C!)
    
    // Button-specific properties
    lv_obj_t* label;        // Label object สำหรับข้อความ
    
    struct {
        char* text;         // Button text
        const lv_font_t* font;
        lv_color_t text_color;
    } button_props;
};

// ===== Button Methods (คล้าง CButton methods ใน MFC) =====

/**
 * @brief Constructor - สร้าง button ใหม่
 * @param parent Parent widget (NULL สำหรับ screen root)
 * @return Pointer to created button
 */
LVButton* lvbutton_create(LVWidget* parent);

/**
 * @brief Destructor - ทำลาย button
 * @param button Button to destroy
 */
void lvbutton_destroy(LVButton* button);

/**
 * @brief Set button text (คล้าง SetWindowText ใน MFC)
 * @param button Button widget
 * @param text Text to display
 */
void lvbutton_set_text(LVButton* button, const char* text);

/**
 * @brief Get button text (คล้าง GetWindowText ใน MFC)
 * @param button Button widget
 * @return Button text (do not free)
 */
const char* lvbutton_get_text(LVButton* button);

/**
 * @brief Set button text font
 * @param button Button widget
 * @param font Font to use
 */
void lvbutton_set_font(LVButton* button, const lv_font_t* font);

/**
 * @brief Set button text color
 * @param button Button widget
 * @param color Text color
 */
void lvbutton_set_text_color(LVButton* button, lv_color_t color);

/**
 * @brief Cast LVButton* to LVWidget* (safe upcast)
 * คล้าง dynamic_cast ใน C++
 */
#define LVBUTTON_TO_WIDGET(btn) ((LVWidget*)(btn))

/**
 * @brief Cast LVWidget* to LVButton* (downcast - ต้องระวัง!)
 * ใช้เฉพาะเมื่อแน่ใจว่า widget นั้นเป็น button
 */
#define WIDGET_TO_LVBUTTON(widget) ((LVButton*)(widget))

#ifdef __cplusplus
}
#endif

#endif // LVGL_BUTTON_H
