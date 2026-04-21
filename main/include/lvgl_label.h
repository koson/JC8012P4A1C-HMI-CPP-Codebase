#ifndef LVGL_LABEL_H
#define LVGL_LABEL_H

#include "lvgl_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Label Widget Class - คล้าย CStatic ใน MFC
 * 
 * Derived class จาก LVWidget สำหรับ Label/Text Display
 */

typedef struct LVLabel LVLabel;

/**
 * @brief Label Structure - inherit จาก LVWidget
 */
struct LVLabel {
    LVWidget base;          // Base class
    
    // Label-specific properties
    struct {
        char* text;
        const lv_font_t* font;
        lv_color_t text_color;
        lv_label_long_mode_t long_mode;  // Text wrapping mode
    } label_props;
};

// ===== Label Methods (คล้าง CStatic methods ใน MFC) =====

/**
 * @brief Constructor - สร้าง label ใหม่
 */
LVLabel* lvlabel_create(LVWidget* parent);

/**
 * @brief Destructor - ทำลาย label
 */
void lvlabel_destroy(LVLabel* label);

/**
 * @brief Set label text
 */
void lvlabel_set_text(LVLabel* label, const char* text);

/**
 * @brief Set label text with format (คล้าง printf)
 */
void lvlabel_set_text_fmt(LVLabel* label, const char* fmt, ...);

/**
 * @brief Get label text
 */
const char* lvlabel_get_text(LVLabel* label);

/**
 * @brief Set label font
 */
void lvlabel_set_font(LVLabel* label, const lv_font_t* font);

/**
 * @brief Set label text color
 */
void lvlabel_set_text_color(LVLabel* label, lv_color_t color);

/**
 * @brief Set long text mode (wrap, scroll, etc.)
 */
void lvlabel_set_long_mode(LVLabel* label, lv_label_long_mode_t mode);

/**
 * @brief Cast macros
 */
#define LVLABEL_TO_WIDGET(lbl) ((LVWidget*)(lbl))
#define WIDGET_TO_LVLABEL(widget) ((LVLabel*)(widget))

#ifdef __cplusplus
}
#endif

#endif // LVGL_LABEL_H
