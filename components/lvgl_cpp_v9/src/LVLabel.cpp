#include "../include/LVLabel.hpp"

// ===== Constructor / Destructor =====

LVLabel::LVLabel(LVWidget* parent)
    : LVWidget(parent, lv_label_create(parent ? parent->obj() : lv_scr_act()))
    , m_text("Label")
    , m_font(&lv_font_montserrat_14)
    , m_textColor(lv_color_white())
    , m_longMode(LV_LABEL_LONG_WRAP)
{
    if (!m_obj) {
        ESP_LOGE(TAG, "Failed to create label object");
        return;
    }
    
    // Set default text
    lv_label_set_text(m_obj, m_text.c_str());
    
    ESP_LOGI(TAG, "Label created: %p (text: %s)", this, m_text.c_str());
}

LVLabel::~LVLabel()
{
    ESP_LOGI(TAG, "Destroying label: %p", this);
    // Base class destructor จะลบ m_obj อัตโนมัติ
}

// ===== Label Methods =====

LVLabel* LVLabel::setText(const std::string& text)
{
    m_text = text;
    
    if (m_obj) {
        lv_label_set_text(m_obj, m_text.c_str());
    }
    return this;
}

LVLabel* LVLabel::setText(const char* text)
{
    if (text) {
        setText(std::string(text));
    }
    return this;
}

void LVLabel::setFont(const lv_font_t* font)
{
    if (!font || !m_obj) return;
    
    m_font = font;
    lv_obj_set_style_text_font(m_obj, font, 0);
}

void LVLabel::setTextColor(lv_color_t color)
{
    if (!m_obj) return;
    
    m_textColor = color;
    lv_obj_set_style_text_color(m_obj, color, 0);
}

void LVLabel::setLongMode(lv_label_long_mode_t mode)
{
    if (!m_obj) return;
    
    m_longMode = mode;
    lv_label_set_long_mode(m_obj, mode);
}

 