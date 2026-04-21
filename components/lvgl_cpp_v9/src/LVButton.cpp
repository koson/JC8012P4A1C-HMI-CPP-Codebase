#include "../include/LVButton.hpp"

// ===== Constructor / Destructor =====

LVButton::LVButton(LVWidget* parent)
    : LVWidget(parent, lv_btn_create(parent ? parent->obj() : lv_scr_act()))
    , m_label(nullptr)
    , m_text("Button")
    , m_font(&lv_font_montserrat_14)
    , m_textColor(lv_color_white())
{
    if (!m_obj) {
        ESP_LOGE(TAG, "Failed to create button object");
        return;
    }
    
    // สร้าง label สำหรับข้อความ
    m_label = lv_label_create(m_obj);
    if (m_label) {
        lv_obj_center(m_label);
        lv_label_set_text(m_label, m_text.c_str());
    }
    
    // Set default size
    lv_obj_set_size(m_obj, 120, 50);
    
    ESP_LOGI(TAG, "Button created: %p (text: %s)", this, m_text.c_str());
}

LVButton::~LVButton()
{
    ESP_LOGI(TAG, "Destroying button: %p", this);
    // Base class destructor จะลบ m_obj (และ m_label ด้วย) อัตโนมัติ
}

// ===== Button Methods =====

LVButton* LVButton::setText(const std::string& text)
{
    m_text = text;
    
    if (m_label) {
        lv_label_set_text(m_label, m_text.c_str());
        lv_obj_center(m_label);  // Center text
    }
    return this;
}

LVButton* LVButton::setText(const char* text)
{
    if (text) {
        setText(std::string(text));
    }
    return this;
}

void LVButton::setFont(const lv_font_t* font)
{
    if (!font || !m_label) return;
    
    m_font = font;
    lv_obj_set_style_text_font(m_label, font, 0);
}

void LVButton::setTextColor(lv_color_t color)
{
    if (!m_label) return;
    
    m_textColor = color;
    lv_obj_set_style_text_color(m_label, color, 0);
}

// ===== Virtual Event Handler =====

void LVButton::handleClicked(lv_event_t* event)
{
    // Button-specific click behavior
    ESP_LOGI(TAG, "Button clicked: %p (text: %s)", this, m_text.c_str());
    
    // เรียก base class implementation (ถ้าต้องการ)
    LVWidget::handleClicked(event);
}
