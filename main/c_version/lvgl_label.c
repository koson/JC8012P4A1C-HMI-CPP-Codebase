#include "include/lvgl_label.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static const char *TAG = "LVLabel";

// ===== Label Virtual Function Table =====

static void label_destroy(LVWidget* widget);

// Label vtable
static const LVWidgetVTable label_vtable = {
    .destroy = label_destroy,
    .set_size = NULL,
    .set_pos = NULL,
    .set_align = NULL,
    .show = NULL,
    .hide = NULL,
    .enable = NULL,
    .disable = NULL,
    .on_clicked = NULL,
    .on_value_changed = NULL,
    .on_focused = NULL,
    .on_defocused = NULL,
};

// ===== Constructor / Destructor =====

LVLabel* lvlabel_create(LVWidget* parent)
{
    LVLabel* label = (LVLabel*)malloc(sizeof(LVLabel));
    if (!label) {
        ESP_LOGE(TAG, "Failed to allocate memory for label");
        return NULL;
    }
    
    // สร้าง LVGL label object
    lv_obj_t* label_obj = lv_label_create(parent ? parent->obj : lv_scr_act());
    if (!label_obj) {
        ESP_LOGE(TAG, "Failed to create LVGL label object");
        free(label);
        return NULL;
    }
    
    // Initialize base class
    lvwidget_init(&label->base, parent, label_obj);
    
    // Override vtable
    label->base.vtable = &label_vtable;
    
    // Initialize label-specific properties
    label->label_props.text = NULL;
    label->label_props.font = &lv_font_montserrat_14;
    label->label_props.text_color = lv_color_white();
    label->label_props.long_mode = LV_LABEL_LONG_WRAP;
    
    // Set default text
    lv_label_set_text(label_obj, "Label");
    
    ESP_LOGI(TAG, "Label created: %p", label);
    
    return label;
}

void lvlabel_destroy(LVLabel* label)
{
    if (!label) return;
    
    if (label->base.vtable && label->base.vtable->destroy) {
        label->base.vtable->destroy(&label->base);
    }
}

// ===== Label Virtual Function Implementations =====

static void label_destroy(LVWidget* widget)
{
    if (!widget) return;
    
    LVLabel* label = WIDGET_TO_LVLABEL(widget);
    
    ESP_LOGI(TAG, "Destroying label: %p", label);
    
    // Free label-specific resources
    if (label->label_props.text) {
        free(label->label_props.text);
        label->label_props.text = NULL;
    }
    
    // ลบ LVGL object
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
    }
    
    memset(label, 0, sizeof(LVLabel));
    free(label);
}

// ===== Label-Specific Methods =====

void lvlabel_set_text(LVLabel* label, const char* text)
{
    if (!label || !label->base.obj) return;
    
    // Free old text
    if (label->label_props.text) {
        free(label->label_props.text);
    }
    
    // Copy new text
    label->label_props.text = text ? strdup(text) : NULL;
    
    // Set text to LVGL label
    lv_label_set_text(label->base.obj, text ? text : "");
}

void lvlabel_set_text_fmt(LVLabel* label, const char* fmt, ...)
{
    if (!label || !label->base.obj || !fmt) return;
    
    // Format text
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Set formatted text
    lvlabel_set_text(label, buffer);
}

const char* lvlabel_get_text(LVLabel* label)
{
    return label ? label->label_props.text : NULL;
}

void lvlabel_set_font(LVLabel* label, const lv_font_t* font)
{
    if (!label || !label->base.obj || !font) return;
    
    label->label_props.font = font;
    lv_obj_set_style_text_font(label->base.obj, font, 0);
}

void lvlabel_set_text_color(LVLabel* label, lv_color_t color)
{
    if (!label || !label->base.obj) return;
    
    label->label_props.text_color = color;
    lv_obj_set_style_text_color(label->base.obj, color, 0);
}

void lvlabel_set_long_mode(LVLabel* label, lv_label_long_mode_t mode)
{
    if (!label || !label->base.obj) return;
    
    label->label_props.long_mode = mode;
    lv_label_set_long_mode(label->base.obj, mode);
}
