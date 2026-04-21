#include "../include/LVWidget.hpp"
#include <utility>

// ===== Constructor / Destructor =====

LVWidget::LVWidget(LVWidget* parent)
    : m_obj(nullptr)
    , m_parent(parent)
    , m_x(0), m_y(0)
    , m_width(0), m_height(0)
    , m_visible(true)
    , m_enabled(true)
    , m_userData(nullptr)
{
    // สร้าง LVGL base object
    m_obj = lv_obj_create(parent ? parent->obj() : lv_scr_act());
    
    if (!m_obj) {
        ESP_LOGE(TAG, "Failed to create LVGL object");
        return;
    }
    
    initialize();
    
    ESP_LOGI(TAG, "Widget created: %p", this);
}

LVWidget::LVWidget(LVWidget* parent, lv_obj_t* obj)
    : m_obj(obj)
    , m_parent(parent)
    , m_x(0), m_y(0)
    , m_width(0), m_height(0)
    , m_visible(true)
    , m_enabled(true)
    , m_userData(nullptr)
{
    initialize();
}

LVWidget::~LVWidget()
{
    ESP_LOGI(TAG, "Destroying widget: %p", this);
    
    // ลบ LVGL object (จะลบ children อัตโนมัติ)
    if (m_obj) {
        lv_obj_del(m_obj);
        m_obj = nullptr;
    }
}

// ===== Move Semantics (C++11) =====

LVWidget::LVWidget(LVWidget&& other) noexcept
    : m_obj(other.m_obj)
    , m_parent(other.m_parent)
    , m_x(other.m_x), m_y(other.m_y)
    , m_width(other.m_width), m_height(other.m_height)
    , m_visible(other.m_visible)
    , m_enabled(other.m_enabled)
    , m_onClicked(std::move(other.m_onClicked))
    , m_onValueChanged(std::move(other.m_onValueChanged))
    , m_onFocused(std::move(other.m_onFocused))
    , m_onDefocused(std::move(other.m_onDefocused))
    , m_userData(other.m_userData)
{
    // Transfer ownership
    other.m_obj = nullptr;
    other.m_parent = nullptr;
    other.m_userData = nullptr;
    
    // Update user data in LVGL object
    if (m_obj) {
        lv_obj_set_user_data(m_obj, this);
    }
}

LVWidget& LVWidget::operator=(LVWidget&& other) noexcept
{
    if (this != &other) {
        // ลบ object เดิม
        if (m_obj) {
            lv_obj_del(m_obj);
        }
        
        // Transfer ownership
        m_obj = other.m_obj;
        m_parent = other.m_parent;
        m_x = other.m_x;
        m_y = other.m_y;
        m_width = other.m_width;
        m_height = other.m_height;
        m_visible = other.m_visible;
        m_enabled = other.m_enabled;
        m_onClicked = std::move(other.m_onClicked);
        m_onValueChanged = std::move(other.m_onValueChanged);
        m_onFocused = std::move(other.m_onFocused);
        m_onDefocused = std::move(other.m_onDefocused);
        m_userData = other.m_userData;
        
        other.m_obj = nullptr;
        other.m_parent = nullptr;
        other.m_userData = nullptr;
        
        // Update user data
        if (m_obj) {
            lv_obj_set_user_data(m_obj, this);
        }
    }
    return *this;
}

// ===== Initialization =====

void LVWidget::initialize()
{
    if (!m_obj) return;
    
    // เก็บ pointer ไปยัง C++ object ใน LVGL user data
    lv_obj_set_user_data(m_obj, this);
    
    // ลงทะเบียน event callback
    lv_obj_add_event_cb(m_obj, eventCallback, LV_EVENT_ALL, this);
}

// ===== Property Methods =====

LVWidget* LVWidget::setSize(int32_t width, int32_t height)
{
    if (!m_obj) return this;
    
    m_width = width;
    m_height = height;
    lv_obj_set_size(m_obj, width, height);
    return this;
}

void LVWidget::getSize(int32_t& width, int32_t& height) const
{
    width = m_width;
    height = m_height;
}

LVWidget* LVWidget::setPos(int32_t x, int32_t y)
{
    if (!m_obj) return this;
    
    m_x = x;
    m_y = y;
    lv_obj_set_pos(m_obj, x, y);
    return this;
}

void LVWidget::getPos(int32_t& x, int32_t& y) const
{
    x = m_x;
    y = m_y;
}

LVWidget* LVWidget::setAlign(lv_align_t align, int32_t x_offset, int32_t y_offset)
{
    if (!m_obj) return this;
    lv_obj_align(m_obj, align, x_offset, y_offset);
    return this;
}

LVWidget* LVWidget::show()
{
    if (!m_obj) return this;
    
    m_visible = true;
    lv_obj_clear_flag(m_obj, LV_OBJ_FLAG_HIDDEN);
    return this;
}

LVWidget* LVWidget::hide()
{
    if (!m_obj) return this;
    
    m_visible = false;
    lv_obj_add_flag(m_obj, LV_OBJ_FLAG_HIDDEN);
    return this;
}

LVWidget* LVWidget::setVisible(bool visible)
{
    if (visible) {
        show();
    } else {
        hide();
    }
    return this;
}

LVWidget* LVWidget::enable()
{
    if (!m_obj) return this;
    
    m_enabled = true;
    lv_obj_clear_state(m_obj, LV_STATE_DISABLED);
    return this;
}

LVWidget* LVWidget::disable()
{
    if (!m_obj) return this;
    
    m_enabled = false;
    lv_obj_add_state(m_obj, LV_STATE_DISABLED);
    return this;
}

LVWidget* LVWidget::setEnabled(bool enabled)
{
    if (enabled) {
        enable();
    } else {
        disable();
    }
    return this;
}

// ===== Style Methods (Fluent API) =====

LVWidget* LVWidget::setBackgroundColor(lv_color_t color)
{
    if (m_obj) {
        lv_obj_set_style_bg_color(m_obj, color, 0);
    }
    return this;  // Return this สำหรับ method chaining
}

LVWidget* LVWidget::setBorderColor(lv_color_t color)
{
    if (m_obj) {
        lv_obj_set_style_border_color(m_obj, color, 0);
    }
    return this;
}

LVWidget* LVWidget::setBorderWidth(int32_t width)
{
    if (m_obj) {
        lv_obj_set_style_border_width(m_obj, width, 0);
    }
    return this;
}

LVWidget* LVWidget::setRadius(int32_t radius)
{
    if (m_obj) {
        lv_obj_set_style_radius(m_obj, radius, 0);
    }
    return this;
}

// ===== Virtual Event Handlers (Default implementations) =====

void LVWidget::handleClicked(lv_event_t* event)
{
    // Default: do nothing (override ใน derived class)
    ESP_LOGI(TAG, "Widget clicked: %p", this);
}

void LVWidget::handleValueChanged(lv_event_t* event)
{
    // Default: do nothing
}

void LVWidget::handleFocused(lv_event_t* event)
{
    // Default: do nothing
}

void LVWidget::handleDefocused(lv_event_t* event)
{
    // Default: do nothing
}

// ===== Event Callback (Static) =====

void LVWidget::eventCallback(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    
    // ดึง C++ object pointer จาก LVGL user data
    LVWidget* widget = static_cast<LVWidget*>(lv_obj_get_user_data(obj));
    if (!widget) return;
    
    // Dispatch event
    switch (code) {
        case LV_EVENT_CLICKED:
            // 1. เรียก user handler (std::function - อาจเป็น lambda)
            if (widget->m_onClicked) {
                widget->m_onClicked(widget, e);
            }
            // 2. เรียก virtual method (polymorphism)
            widget->handleClicked(e);
            break;
            
        case LV_EVENT_VALUE_CHANGED:
            if (widget->m_onValueChanged) {
                widget->m_onValueChanged(widget, e);
            }
            widget->handleValueChanged(e);
            break;
            
        case LV_EVENT_FOCUSED:
            if (widget->m_onFocused) {
                widget->m_onFocused(widget, e);
            }
            widget->handleFocused(e);
            break;
            
        case LV_EVENT_DEFOCUSED:
            if (widget->m_onDefocused) {
                widget->m_onDefocused(widget, e);
            }
            widget->handleDefocused(e);
            break;
            
        default:
            break;
    }
}
