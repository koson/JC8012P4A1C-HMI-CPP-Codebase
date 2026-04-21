#ifndef LVGL_WIDGET_H
#define LVGL_WIDGET_H

#include "lvgl.h"
#include "esp_log.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Base Widget Class - คล้ายกับ CWnd ใน MFC
 * 
 * นี่คือ Base Class สำหรับ Widget ทั้งหมด
 * ใช้หลักการ OOP ใน C ด้วย Virtual Function Table (vtable)
 */

// Forward declarations
typedef struct LVWidget LVWidget;
typedef struct LVWidgetVTable LVWidgetVTable;

/**
 * @brief Event callback function pointer type
 * คล้าย event handler ใน MFC/WinForms
 */
typedef void (*LVEventHandler)(LVWidget* widget, lv_event_t* event);

/**
 * @brief Virtual Function Table - เหมือน vtable ใน C++
 * 
 * ใช้สำหรับ polymorphism แบบ OOP
 */
struct LVWidgetVTable {
    // Virtual destructor
    void (*destroy)(LVWidget* widget);
    
    // Virtual methods
    void (*set_size)(LVWidget* widget, int32_t width, int32_t height);
    void (*set_pos)(LVWidget* widget, int32_t x, int32_t y);
    void (*set_align)(LVWidget* widget, lv_align_t align, int32_t x_offset, int32_t y_offset);
    void (*show)(LVWidget* widget);
    void (*hide)(LVWidget* widget);
    void (*enable)(LVWidget* widget);
    void (*disable)(LVWidget* widget);
    
    // Event handling
    void (*on_clicked)(LVWidget* widget, lv_event_t* event);
    void (*on_value_changed)(LVWidget* widget, lv_event_t* event);
    void (*on_focused)(LVWidget* widget, lv_event_t* event);
    void (*on_defocused)(LVWidget* widget, lv_event_t* event);
};

/**
 * @brief Base Widget Structure - คล้าย CWnd ใน MFC
 * 
 * นี่คือ base class ที่ทุก widget จะ inherit จาก
 */
struct LVWidget {
    // "Virtual" function table pointer (เหมือน vtable ใน C++)
    const LVWidgetVTable* vtable;
    
    // LVGL object handle
    lv_obj_t* obj;
    
    // Parent widget (สำหรับสร้าง hierarchy)
    LVWidget* parent;
    
    // User data pointer (คล้าย SetWindowLongPtr ใน Win32)
    void* user_data;
    
    // Properties
    struct {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
        bool visible;
        bool enabled;
    } properties;
    
    // Event handlers (คล้าย message map ใน MFC)
    struct {
        LVEventHandler on_clicked;
        LVEventHandler on_value_changed;
        LVEventHandler on_focused;
        LVEventHandler on_defocused;
        LVEventHandler on_custom;
    } event_handlers;
};

// ===== Base Widget Methods (คล้าง CWnd methods ใน MFC) =====

/**
 * @brief Constructor - สร้าง widget ใหม่
 * @param parent Parent widget (NULL สำหรับ screen root)
 * @return Pointer to created widget
 */
LVWidget* lvwidget_create(LVWidget* parent);

/**
 * @brief Destructor - ทำลาย widget
 * @param widget Widget to destroy
 */
void lvwidget_destroy(LVWidget* widget);

/**
 * @brief Initialize base widget (ใช้ภายใน - คล้าย protected constructor)
 * @param widget Widget instance
 * @param parent Parent widget
 * @param obj LVGL object
 */
void lvwidget_init(LVWidget* widget, LVWidget* parent, lv_obj_t* obj);

// ===== Property Methods (คล้าง Get/Set methods ใน MFC) =====

void lvwidget_set_size(LVWidget* widget, int32_t width, int32_t height);
void lvwidget_get_size(LVWidget* widget, int32_t* width, int32_t* height);

void lvwidget_set_pos(LVWidget* widget, int32_t x, int32_t y);
void lvwidget_get_pos(LVWidget* widget, int32_t* x, int32_t* y);

void lvwidget_set_align(LVWidget* widget, lv_align_t align, int32_t x_offset, int32_t y_offset);

void lvwidget_show(LVWidget* widget);
void lvwidget_hide(LVWidget* widget);

void lvwidget_enable(LVWidget* widget);
void lvwidget_disable(LVWidget* widget);

bool lvwidget_is_visible(LVWidget* widget);
bool lvwidget_is_enabled(LVWidget* widget);

// ===== Style Methods (คล้าง SetWindowLong/SetFont ใน Win32) =====

void lvwidget_set_bg_color(LVWidget* widget, lv_color_t color);
void lvwidget_set_border_color(LVWidget* widget, lv_color_t color);
void lvwidget_set_border_width(LVWidget* widget, int32_t width);
void lvwidget_set_radius(LVWidget* widget, int32_t radius);

// ===== Event Handling Methods (คล้าง message handlers ใน MFC) =====

/**
 * @brief Set event handler for clicked event
 * คล้าย ON_BN_CLICKED ใน MFC
 */
void lvwidget_set_on_clicked(LVWidget* widget, LVEventHandler handler);

/**
 * @brief Set event handler for value changed event
 */
void lvwidget_set_on_value_changed(LVWidget* widget, LVEventHandler handler);

/**
 * @brief Set event handler for focused event
 */
void lvwidget_set_on_focused(LVWidget* widget, LVEventHandler handler);

/**
 * @brief Set event handler for defocused event
 */
void lvwidget_set_on_defocused(LVWidget* widget, LVEventHandler handler);

/**
 * @brief Set custom event handler
 */
void lvwidget_set_on_custom(LVWidget* widget, LVEventHandler handler);

// ===== User Data Methods (คล้าง SetWindowLongPtr ใน Win32) =====

void lvwidget_set_user_data(LVWidget* widget, void* data);
void* lvwidget_get_user_data(LVWidget* widget);

// ===== Internal Event Dispatcher =====

/**
 * @brief Internal event callback for LVGL
 * ใช้สำหรับ dispatch events ไปยัง user handlers
 */
void lvwidget_event_callback(lv_event_t* e);

#ifdef __cplusplus
}
#endif

#endif // LVGL_WIDGET_H
