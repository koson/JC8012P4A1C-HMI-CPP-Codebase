#ifndef LVGL_PANEL_H
#define LVGL_PANEL_H

#include "lvgl_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Panel Widget Class - คล้าย CDialog หรือ Panel ใน WinForms
 * 
 * Derived class จาก LVWidget สำหรับ Container/Panel
 * ใช้สำหรับจัดกลุ่ม widgets
 */

typedef struct LVPanel LVPanel;

/**
 * @brief Panel Structure - inherit จาก LVWidget
 */
struct LVPanel {
    LVWidget base;          // Base class
    
    // Panel-specific properties
    struct {
        lv_layout_t layout;     // Layout type
        lv_flex_flow_t flex_flow;
        int32_t padding;
        int32_t gap;
    } panel_props;
};

// ===== Panel Methods =====

/**
 * @brief Constructor - สร้าง panel ใหม่
 */
LVPanel* lvpanel_create(LVWidget* parent);

/**
 * @brief Destructor - ทำลาย panel
 */
void lvpanel_destroy(LVPanel* panel);

/**
 * @brief Set panel layout to flex (column or row)
 */
void lvpanel_set_flex_flow(LVPanel* panel, lv_flex_flow_t flow);

/**
 * @brief Set panel padding
 */
void lvpanel_set_padding(LVPanel* panel, int32_t padding);

/**
 * @brief Set gap between child items
 */
void lvpanel_set_gap(LVPanel* panel, int32_t gap);

/**
 * @brief Enable/disable scrolling
 */
void lvpanel_set_scrollable(LVPanel* panel, bool scrollable);

/**
 * @brief Cast macros
 */
#define LVPANEL_TO_WIDGET(pnl) ((LVWidget*)(pnl))
#define WIDGET_TO_LVPANEL(widget) ((LVPanel*)(widget))

#ifdef __cplusplus
}
#endif

#endif // LVGL_PANEL_H
