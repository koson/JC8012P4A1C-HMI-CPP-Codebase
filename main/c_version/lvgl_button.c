#include "include/lvgl_button.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "LVButton";

// ===== Button Virtual Function Table =====
// Override base class methods

static void button_destroy(LVWidget* widget);
static void button_on_clicked(LVWidget* widget, lv_event_t* event);

// Button vtable (override base class vtable)
static const LVWidgetVTable button_vtable = {
    .destroy = button_destroy,
    .set_size = NULL,           // ใช้ default จาก base class
    .set_pos = NULL,            // ใช้ default จาก base class
    .set_align = NULL,          // ใช้ default จาก base class
    .show = NULL,               // ใช้ default จาก base class
    .hide = NULL,               // ใช้ default จาก base class
    .enable = NULL,             // ใช้ default จาก base class
    .disable = NULL,            // ใช้ default จาก base class
    .on_clicked = button_on_clicked,
    .on_value_changed = NULL,
    .on_focused = NULL,
    .on_defocused = NULL,
};

// ===== Constructor / Destructor =====

LVButton* lvbutton_create(LVWidget* parent)
{
    // Allocate memory for button (include base class)
    LVButton* button = (LVButton*)malloc(sizeof(LVButton));
    if (!button) {
        ESP_LOGE(TAG, "Failed to allocate memory for button");
        return NULL;
    }
    
    // สร้าง LVGL button object
    lv_obj_t* btn_obj = lv_btn_create(parent ? parent->obj : lv_scr_act());
    if (!btn_obj) {
        ESP_LOGE(TAG, "Failed to create LVGL button object");
        free(button);
        return NULL;
    }
    
    // Initialize base class
    lvwidget_init(&button->base, parent, btn_obj);
    
    // Override vtable to button vtable (polymorphism!)
    button->base.vtable = &button_vtable;
    
    // Create label for text
    button->label = lv_label_create(btn_obj);
    if (button->label) {
        lv_obj_center(button->label);
        lv_label_set_text(button->label, "Button");
    }
    
    // Initialize button-specific properties
    button->button_props.text = NULL;
    button->button_props.font = &lv_font_montserrat_14;
    button->button_props.text_color = lv_color_white();
    
    // Set default button style
    lv_obj_set_size(btn_obj, 120, 50);
    
    ESP_LOGI(TAG, "Button created: %p", button);
    
    return button;
}

void lvbutton_destroy(LVButton* button)
{
    if (!button) return;
    
    // เรียก virtual destructor ผ่าน vtable (polymorphism)
    if (button->base.vtable && button->base.vtable->destroy) {
        button->base.vtable->destroy(&button->base);
    }
}

// ===== Button Virtual Function Implementations =====

static void button_destroy(LVWidget* widget)
{
    if (!widget) return;
    
    // Downcast to button
    LVButton* button = WIDGET_TO_LVBUTTON(widget);
    
    ESP_LOGI(TAG, "Destroying button: %p", button);
    
    // Free button-specific resources
    if (button->button_props.text) {
        free(button->button_props.text);
        button->button_props.text = NULL;
    }
    
    // ลบ LVGL objects
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
    }
    
    // ล้างค่า
    memset(button, 0, sizeof(LVButton));
    
    // Free memory
    free(button);
}

static void button_on_clicked(LVWidget* widget, lv_event_t* event)
{
    LVButton* button = WIDGET_TO_LVBUTTON(widget);
    ESP_LOGI(TAG, "Button clicked: %p (Text: %s)", button, 
             button->button_props.text ? button->button_props.text : "NULL");
}

// ===== Button-Specific Methods =====

void lvbutton_set_text(LVButton* button, const char* text)
{
    if (!button || !button->label) return;
    
    // Free old text
    if (button->button_props.text) {
        free(button->button_props.text);
    }
    
    // Copy new text
    button->button_props.text = text ? strdup(text) : NULL;
    
    // Set text to label
    lv_label_set_text(button->label, text ? text : "");
    
    // Center label
    lv_obj_center(button->label);
}

const char* lvbutton_get_text(LVButton* button)
{
    return button ? button->button_props.text : NULL;
}

void lvbutton_set_font(LVButton* button, const lv_font_t* font)
{
    if (!button || !button->label || !font) return;
    
    button->button_props.font = font;
    lv_obj_set_style_text_font(button->label, font, 0);
}

void lvbutton_set_text_color(LVButton* button, lv_color_t color)
{
    if (!button || !button->label) return;
    
    button->button_props.text_color = color;
    lv_obj_set_style_text_color(button->label, color, 0);
}
