#include "../include/LVPanel.hpp"

// ===== Constructor / Destructor =====

LVPanel::LVPanel(LVWidget* parent)
    : LVWidget(parent, lv_obj_create(parent ? parent->obj() : lv_scr_act()))
    , m_flexFlow(LV_FLEX_FLOW_ROW)
    , m_padding(10)
    , m_gap(5)
{
    if (!m_obj) {
        ESP_LOGE(TAG, "Failed to create panel object");
        return;
    }
    
    // Set default size
    lv_obj_set_size(m_obj, 200, 150);
    
    // Set default padding
    lv_obj_set_style_pad_all(m_obj, m_padding, 0);
    
    ESP_LOGI(TAG, "Panel created: %p", this);
}

LVPanel::~LVPanel()
{
    ESP_LOGI(TAG, "Destroying panel: %p", this);
    // Base class destructor จะลบ m_obj และ children อัตโนมัติ
}

// ===== Panel Methods =====

void LVPanel::setFlexFlow(lv_flex_flow_t flow)
{
    if (!m_obj) return;
    
    m_flexFlow = flow;
    lv_obj_set_flex_flow(m_obj, flow);
}

void LVPanel::setPadding(int32_t padding)
{
    if (!m_obj) return;
    
    m_padding = padding;
    lv_obj_set_style_pad_all(m_obj, padding, 0);
}

void LVPanel::setGap(int32_t gap)
{
    if (!m_obj) return;
    
    m_gap = gap;
    lv_obj_set_style_pad_gap(m_obj, gap, 0);
}

void LVPanel::setScrollable(bool scrollable)
{
    if (!m_obj) return;
    
    if (scrollable) {
        lv_obj_set_scrollbar_mode(m_obj, LV_SCROLLBAR_MODE_AUTO);
    } else {
        lv_obj_set_scrollbar_mode(m_obj, LV_SCROLLBAR_MODE_OFF);
    }
}
