#include "include/lvgl_widget.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "LVWidget";

// ===== Default Virtual Function Table =====

static void default_destroy(LVWidget* widget);
static void default_set_size(LVWidget* widget, int32_t width, int32_t height);
static void default_set_pos(LVWidget* widget, int32_t x, int32_t y);
static void default_set_align(LVWidget* widget, lv_align_t align, int32_t x_offset, int32_t y_offset);
static void default_show(LVWidget* widget);
static void default_hide(LVWidget* widget);
static void default_enable(LVWidget* widget);
static void default_disable(LVWidget* widget);

// Default vtable (เหมือน default implementation ของ virtual functions)
static const LVWidgetVTable default_vtable = {
    .destroy = default_destroy,
    .set_size = default_set_size,
    .set_pos = default_set_pos,
    .set_align = default_set_align,
    .show = default_show,
    .hide = default_hide,
    .enable = default_enable,
    .disable = default_disable,
    .on_clicked = NULL,
    .on_value_changed = NULL,
    .on_focused = NULL,
    .on_defocused = NULL,
};

// ===== Constructor / Destructor =====

LVWidget* lvwidget_create(LVWidget* parent)
{
    LVWidget* widget = (LVWidget*)malloc(sizeof(LVWidget));
    if (!widget) {
        ESP_LOGE(TAG, "Failed to allocate memory for widget");
        return NULL;
    }
    
    // สร้าง LVGL object (base object)
    lv_obj_t* obj = lv_obj_create(parent ? parent->obj : lv_scr_act());
    if (!obj) {
        ESP_LOGE(TAG, "Failed to create LVGL object");
        free(widget);
        return NULL;
    }
    
    // Initialize widget
    lvwidget_init(widget, parent, obj);
    
    return widget;
}

void lvwidget_destroy(LVWidget* widget)
{
    if (!widget) return;
    
    // เรียก virtual destructor (polymorphism)
    if (widget->vtable && widget->vtable->destroy) {
        widget->vtable->destroy(widget);
    }
}

void lvwidget_init(LVWidget* widget, LVWidget* parent, lv_obj_t* obj)
{
    if (!widget) return;
    
    // Initialize structure
    memset(widget, 0, sizeof(LVWidget));
    
    // Set vtable (default)
    widget->vtable = &default_vtable;
    
    // Set LVGL object
    widget->obj = obj;
    widget->parent = parent;
    
    // Set user data in LVGL object (เก็บ pointer ไปยัง widget struct)
    lv_obj_set_user_data(obj, widget);
    
    // Set default properties
    widget->properties.visible = true;
    widget->properties.enabled = true;
    
    // Add event callback to LVGL object
    lv_obj_add_event_cb(obj, lvwidget_event_callback, LV_EVENT_ALL, widget);
    
    ESP_LOGI(TAG, "Widget initialized: %p", widget);
}

// ===== Default Virtual Function Implementations =====

static void default_destroy(LVWidget* widget)
{
    if (!widget) return;
    
    ESP_LOGI(TAG, "Destroying widget: %p", widget);
    
    // ลบ LVGL object
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
    }
    
    // ล้างค่า
    memset(widget, 0, sizeof(LVWidget));
    
    // Free memory
    free(widget);
}

static void default_set_size(LVWidget* widget, int32_t width, int32_t height)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_set_size(widget->obj, width, height);
    widget->properties.width = width;
    widget->properties.height = height;
}

static void default_set_pos(LVWidget* widget, int32_t x, int32_t y)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_set_pos(widget->obj, x, y);
    widget->properties.x = x;
    widget->properties.y = y;
}

static void default_set_align(LVWidget* widget, lv_align_t align, int32_t x_offset, int32_t y_offset)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_align(widget->obj, align, x_offset, y_offset);
}

static void default_show(LVWidget* widget)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
    widget->properties.visible = true;
}

static void default_hide(LVWidget* widget)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_add_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);
    widget->properties.visible = false;
}

static void default_enable(LVWidget* widget)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_clear_state(widget->obj, LV_STATE_DISABLED);
    widget->properties.enabled = true;
}

static void default_disable(LVWidget* widget)
{
    if (!widget || !widget->obj) return;
    
    lv_obj_add_state(widget->obj, LV_STATE_DISABLED);
    widget->properties.enabled = false;
}

// ===== Public Property Methods =====

void lvwidget_set_size(LVWidget* widget, int32_t width, int32_t height)
{
    if (!widget || !widget->vtable || !widget->vtable->set_size) return;
    widget->vtable->set_size(widget, width, height);
}

void lvwidget_get_size(LVWidget* widget, int32_t* width, int32_t* height)
{
    if (!widget) return;
    if (width) *width = widget->properties.width;
    if (height) *height = widget->properties.height;
}

void lvwidget_set_pos(LVWidget* widget, int32_t x, int32_t y)
{
    if (!widget || !widget->vtable || !widget->vtable->set_pos) return;
    widget->vtable->set_pos(widget, x, y);
}

void lvwidget_get_pos(LVWidget* widget, int32_t* x, int32_t* y)
{
    if (!widget) return;
    if (x) *x = widget->properties.x;
    if (y) *y = widget->properties.y;
}

void lvwidget_set_align(LVWidget* widget, lv_align_t align, int32_t x_offset, int32_t y_offset)
{
    if (!widget || !widget->vtable || !widget->vtable->set_align) return;
    widget->vtable->set_align(widget, align, x_offset, y_offset);
}

void lvwidget_show(LVWidget* widget)
{
    if (!widget || !widget->vtable || !widget->vtable->show) return;
    widget->vtable->show(widget);
}

void lvwidget_hide(LVWidget* widget)
{
    if (!widget || !widget->vtable || !widget->vtable->hide) return;
    widget->vtable->hide(widget);
}

void lvwidget_enable(LVWidget* widget)
{
    if (!widget || !widget->vtable || !widget->vtable->enable) return;
    widget->vtable->enable(widget);
}

void lvwidget_disable(LVWidget* widget)
{
    if (!widget || !widget->vtable || !widget->vtable->disable) return;
    widget->vtable->disable(widget);
}

bool lvwidget_is_visible(LVWidget* widget)
{
    return widget ? widget->properties.visible : false;
}

bool lvwidget_is_enabled(LVWidget* widget)
{
    return widget ? widget->properties.enabled : false;
}

// ===== Style Methods =====

void lvwidget_set_bg_color(LVWidget* widget, lv_color_t color)
{
    if (!widget || !widget->obj) return;
    lv_obj_set_style_bg_color(widget->obj, color, 0);
}

void lvwidget_set_border_color(LVWidget* widget, lv_color_t color)
{
    if (!widget || !widget->obj) return;
    lv_obj_set_style_border_color(widget->obj, color, 0);
}

void lvwidget_set_border_width(LVWidget* widget, int32_t width)
{
    if (!widget || !widget->obj) return;
    lv_obj_set_style_border_width(widget->obj, width, 0);
}

void lvwidget_set_radius(LVWidget* widget, int32_t radius)
{
    if (!widget || !widget->obj) return;
    lv_obj_set_style_radius(widget->obj, radius, 0);
}

// ===== Event Handling =====

void lvwidget_set_on_clicked(LVWidget* widget, LVEventHandler handler)
{
    if (!widget) return;
    widget->event_handlers.on_clicked = handler;
}

void lvwidget_set_on_value_changed(LVWidget* widget, LVEventHandler handler)
{
    if (!widget) return;
    widget->event_handlers.on_value_changed = handler;
}

void lvwidget_set_on_focused(LVWidget* widget, LVEventHandler handler)
{
    if (!widget) return;
    widget->event_handlers.on_focused = handler;
}

void lvwidget_set_on_defocused(LVWidget* widget, LVEventHandler handler)
{
    if (!widget) return;
    widget->event_handlers.on_defocused = handler;
}

void lvwidget_set_on_custom(LVWidget* widget, LVEventHandler handler)
{
    if (!widget) return;
    widget->event_handlers.on_custom = handler;
}

// ===== User Data =====

void lvwidget_set_user_data(LVWidget* widget, void* data)
{
    if (!widget) return;
    widget->user_data = data;
}

void* lvwidget_get_user_data(LVWidget* widget)
{
    return widget ? widget->user_data : NULL;
}

// ===== Event Dispatcher (คล้าย WndProc ใน Win32) =====

void lvwidget_event_callback(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);
    
    // ดึง widget pointer จาก user data
    LVWidget* widget = (LVWidget*)lv_obj_get_user_data(obj);
    if (!widget) return;
    
    // Dispatch event ไปยัง handler ที่เหมาะสม (คล้าย message map ใน MFC)
    switch (code) {
        case LV_EVENT_CLICKED:
            if (widget->event_handlers.on_clicked) {
                widget->event_handlers.on_clicked(widget, e);
            }
            // เรียก virtual function ถ้ามี
            if (widget->vtable && widget->vtable->on_clicked) {
                widget->vtable->on_clicked(widget, e);
            }
            break;
            
        case LV_EVENT_VALUE_CHANGED:
            if (widget->event_handlers.on_value_changed) {
                widget->event_handlers.on_value_changed(widget, e);
            }
            if (widget->vtable && widget->vtable->on_value_changed) {
                widget->vtable->on_value_changed(widget, e);
            }
            break;
            
        case LV_EVENT_FOCUSED:
            if (widget->event_handlers.on_focused) {
                widget->event_handlers.on_focused(widget, e);
            }
            if (widget->vtable && widget->vtable->on_focused) {
                widget->vtable->on_focused(widget, e);
            }
            break;
            
        case LV_EVENT_DEFOCUSED:
            if (widget->event_handlers.on_defocused) {
                widget->event_handlers.on_defocused(widget, e);
            }
            if (widget->vtable && widget->vtable->on_defocused) {
                widget->vtable->on_defocused(widget, e);
            }
            break;
            
        default:
            // Custom events
            if (widget->event_handlers.on_custom) {
                widget->event_handlers.on_custom(widget, e);
            }
            break;
    }
}
