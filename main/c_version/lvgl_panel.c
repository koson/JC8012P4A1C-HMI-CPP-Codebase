#include "include/lvgl_panel.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "LVPanel";

// ===== Panel Virtual Function Table =====

static void panel_destroy(LVWidget* widget);

// Panel vtable
static const LVWidgetVTable panel_vtable = {
    .destroy = panel_destroy,
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

LVPanel* lvpanel_create(LVWidget* parent)
{
    LVPanel* panel = (LVPanel*)malloc(sizeof(LVPanel));
    if (!panel) {
        ESP_LOGE(TAG, "Failed to allocate memory for panel");
        return NULL;
    }
    
    // สร้าง LVGL object (base container)
    lv_obj_t* panel_obj = lv_obj_create(parent ? parent->obj : lv_scr_act());
    if (!panel_obj) {
        ESP_LOGE(TAG, "Failed to create LVGL panel object");
        free(panel);
        return NULL;
    }
    
    // Initialize base class
    lvwidget_init(&panel->base, parent, panel_obj);
    
    // Override vtable
    panel->base.vtable = &panel_vtable;
    
    // Initialize panel-specific properties
    panel->panel_props.layout = 0;
    panel->panel_props.flex_flow = LV_FLEX_FLOW_ROW;
    panel->panel_props.padding = 10;
    panel->panel_props.gap = 5;
    
    // Set default panel style
    lv_obj_set_size(panel_obj, 200, 150);
    lv_obj_set_style_pad_all(panel_obj, panel->panel_props.padding, 0);
    
    ESP_LOGI(TAG, "Panel created: %p", panel);
    
    return panel;
}

void lvpanel_destroy(LVPanel* panel)
{
    if (!panel) return;
    
    if (panel->base.vtable && panel->base.vtable->destroy) {
        panel->base.vtable->destroy(&panel->base);
    }
}

// ===== Panel Virtual Function Implementations =====

static void panel_destroy(LVWidget* widget)
{
    if (!widget) return;
    
    LVPanel* panel = WIDGET_TO_LVPANEL(widget);
    
    ESP_LOGI(TAG, "Destroying panel: %p", panel);
    
    // ลบ LVGL object (จะลบ children ด้วยอัตโนมัติ)
    if (widget->obj) {
        lv_obj_del(widget->obj);
        widget->obj = NULL;
    }
    
    memset(panel, 0, sizeof(LVPanel));
    free(panel);
}

// ===== Panel-Specific Methods =====

void lvpanel_set_flex_flow(LVPanel* panel, lv_flex_flow_t flow)
{
    if (!panel || !panel->base.obj) return;
    
    panel->panel_props.flex_flow = flow;
    lv_obj_set_flex_flow(panel->base.obj, flow);
}

void lvpanel_set_padding(LVPanel* panel, int32_t padding)
{
    if (!panel || !panel->base.obj) return;
    
    panel->panel_props.padding = padding;
    lv_obj_set_style_pad_all(panel->base.obj, padding, 0);
}

void lvpanel_set_gap(LVPanel* panel, int32_t gap)
{
    if (!panel || !panel->base.obj) return;
    
    panel->panel_props.gap = gap;
    lv_obj_set_style_pad_gap(panel->base.obj, gap, 0);
}

void lvpanel_set_scrollable(LVPanel* panel, bool scrollable)
{
    if (!panel || !panel->base.obj) return;
    
    if (scrollable) {
        lv_obj_set_scrollbar_mode(panel->base.obj, LV_SCROLLBAR_MODE_AUTO);
    } else {
        lv_obj_set_scrollbar_mode(panel->base.obj, LV_SCROLLBAR_MODE_OFF);
    }
}
