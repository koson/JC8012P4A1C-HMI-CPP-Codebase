#include "LVStyle.hpp"

LVStyle::LVStyle() : m_initialized(false)
{
    lv_style_init(&m_style);
    m_initialized = true;
}

LVStyle::~LVStyle()
{
    if (m_initialized) {
        lv_style_reset(&m_style);
    }
}

LVStyle::LVStyle(LVStyle&& other) noexcept 
    : m_style(other.m_style), m_initialized(other.m_initialized)
{
    other.m_initialized = false;
}

LVStyle& LVStyle::operator=(LVStyle&& other) noexcept
{
    if (this != &other) {
        if (m_initialized) {
            lv_style_reset(&m_style);
        }
        m_style = other.m_style;
        m_initialized = other.m_initialized;
        other.m_initialized = false;
    }
    return *this;
}

// ========== Size & Layout ==========

LVStyle& LVStyle::setWidth(int32_t width)
{
    lv_style_set_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setHeight(int32_t height)
{
    lv_style_set_height(&m_style, height);
    return *this;
}

LVStyle& LVStyle::setMinWidth(int32_t width)
{
    lv_style_set_min_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setMinHeight(int32_t height)
{
    lv_style_set_min_height(&m_style, height);
    return *this;
}

LVStyle& LVStyle::setMaxWidth(int32_t width)
{
    lv_style_set_max_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setMaxHeight(int32_t height)
{
    lv_style_set_max_height(&m_style, height);
    return *this;
}

LVStyle& LVStyle::setPadding(int32_t all)
{
    lv_style_set_pad_all(&m_style, all);
    return *this;
}

LVStyle& LVStyle::setPadding(int32_t top, int32_t right, int32_t bottom, int32_t left)
{
    lv_style_set_pad_top(&m_style, top);
    lv_style_set_pad_right(&m_style, right);
    lv_style_set_pad_bottom(&m_style, bottom);
    lv_style_set_pad_left(&m_style, left);
    return *this;
}

LVStyle& LVStyle::setPaddingTop(int32_t value)
{
    lv_style_set_pad_top(&m_style, value);
    return *this;
}

LVStyle& LVStyle::setPaddingRight(int32_t value)
{
    lv_style_set_pad_right(&m_style, value);
    return *this;
}

LVStyle& LVStyle::setPaddingBottom(int32_t value)
{
    lv_style_set_pad_bottom(&m_style, value);
    return *this;
}

LVStyle& LVStyle::setPaddingLeft(int32_t value)
{
    lv_style_set_pad_left(&m_style, value);
    return *this;
}

LVStyle& LVStyle::setMargin(int32_t all)
{
    lv_style_set_margin_all(&m_style, all);
    return *this;
}

LVStyle& LVStyle::setMargin(int32_t top, int32_t right, int32_t bottom, int32_t left)
{
    lv_style_set_margin_top(&m_style, top);
    lv_style_set_margin_right(&m_style, right);
    lv_style_set_margin_bottom(&m_style, bottom);
    lv_style_set_margin_left(&m_style, left);
    return *this;
}

// ========== Colors & Appearance ==========

LVStyle& LVStyle::setBackgroundColor(LVColor color)
{
    lv_style_set_bg_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setBackgroundOpacity(lv_opa_t opa)
{
    lv_style_set_bg_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setBackgroundGradientColor(LVColor color)
{
    lv_style_set_bg_grad_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setBackgroundGradientDirection(lv_grad_dir_t dir)
{
    lv_style_set_bg_grad_dir(&m_style, dir);
    return *this;
}

LVStyle& LVStyle::setBorderColor(LVColor color)
{
    lv_style_set_border_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setBorderWidth(int32_t width)
{
    lv_style_set_border_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setBorderOpacity(lv_opa_t opa)
{
    lv_style_set_border_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setBorderSide(lv_border_side_t side)
{
    lv_style_set_border_side(&m_style, side);
    return *this;
}

LVStyle& LVStyle::setOutlineColor(LVColor color)
{
    lv_style_set_outline_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setOutlineWidth(int32_t width)
{
    lv_style_set_outline_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setOutlinePad(int32_t pad)
{
    lv_style_set_outline_pad(&m_style, pad);
    return *this;
}

LVStyle& LVStyle::setOutlineOpacity(lv_opa_t opa)
{
    lv_style_set_outline_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setShadowColor(LVColor color)
{
    lv_style_set_shadow_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setShadowWidth(int32_t width)
{
    lv_style_set_shadow_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setShadowOffset(int32_t x, int32_t y)
{
    lv_style_set_shadow_offset_x(&m_style, x);
    lv_style_set_shadow_offset_y(&m_style, y);
    return *this;
}

LVStyle& LVStyle::setShadowSpread(int32_t spread)
{
    lv_style_set_shadow_spread(&m_style, spread);
    return *this;
}

LVStyle& LVStyle::setShadowOpacity(lv_opa_t opa)
{
    lv_style_set_shadow_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setRadius(int32_t radius)
{
    lv_style_set_radius(&m_style, radius);
    return *this;
}

LVStyle& LVStyle::setOpacity(lv_opa_t opa)
{
    lv_style_set_opa(&m_style, opa);
    return *this;
}

// ========== Text Styling ==========

LVStyle& LVStyle::setTextColor(LVColor color)
{
    lv_style_set_text_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setTextOpacity(lv_opa_t opa)
{
    lv_style_set_text_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setTextFont(const lv_font_t* font)
{
    lv_style_set_text_font(&m_style, font);
    return *this;
}

LVStyle& LVStyle::setTextAlign(lv_text_align_t align)
{
    lv_style_set_text_align(&m_style, align);
    return *this;
}

LVStyle& LVStyle::setTextLetterSpacing(int32_t space)
{
    lv_style_set_text_letter_space(&m_style, space);
    return *this;
}

LVStyle& LVStyle::setTextLineSpacing(int32_t space)
{
    lv_style_set_text_line_space(&m_style, space);
    return *this;
}

LVStyle& LVStyle::setTextDecoration(lv_text_decor_t decor)
{
    lv_style_set_text_decor(&m_style, decor);
    return *this;
}

// ========== Image Styling ==========

LVStyle& LVStyle::setImageOpacity(lv_opa_t opa)
{
    lv_style_set_image_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setImageRecolor(LVColor color)
{
    lv_style_set_image_recolor(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setImageRecolorOpacity(lv_opa_t opa)
{
    lv_style_set_image_recolor_opa(&m_style, opa);
    return *this;
}

// ========== Line Styling ==========

LVStyle& LVStyle::setLineColor(LVColor color)
{
    lv_style_set_line_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setLineWidth(int32_t width)
{
    lv_style_set_line_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setLineOpacity(lv_opa_t opa)
{
    lv_style_set_line_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setLineRounded(bool rounded)
{
    lv_style_set_line_rounded(&m_style, rounded);
    return *this;
}

// ========== Arc Styling ==========

LVStyle& LVStyle::setArcColor(LVColor color)
{
    lv_style_set_arc_color(&m_style, color.raw());
    return *this;
}

LVStyle& LVStyle::setArcWidth(int32_t width)
{
    lv_style_set_arc_width(&m_style, width);
    return *this;
}

LVStyle& LVStyle::setArcOpacity(lv_opa_t opa)
{
    lv_style_set_arc_opa(&m_style, opa);
    return *this;
}

LVStyle& LVStyle::setArcRounded(bool rounded)
{
    lv_style_set_arc_rounded(&m_style, rounded);
    return *this;
}

// ========== Blend Mode & Transform ==========

LVStyle& LVStyle::setBlendMode(lv_blend_mode_t mode)
{
    lv_style_set_blend_mode(&m_style, mode);
    return *this;
}

LVStyle& LVStyle::setTransformAngle(int32_t angle)
{
    lv_style_set_transform_rotation(&m_style, angle);
    return *this;
}

LVStyle& LVStyle::setTransformZoom(int32_t zoom)
{
    lv_style_set_transform_scale(&m_style, zoom);
    return *this;
}

// ========== Transitions ==========

LVStyle& LVStyle::setTransition(const lv_style_transition_dsc_t* trans)
{
    lv_style_set_transition(&m_style, trans);
    return *this;
}

// ========== Application ==========

void LVStyle::applyTo(lv_obj_t* obj, lv_style_selector_t selector)
{
    if (obj && m_initialized) {
        lv_obj_add_style(obj, &m_style, selector);
    }
}

void LVStyle::removeFrom(lv_obj_t* obj, lv_style_selector_t selector)
{
    if (obj && m_initialized) {
        lv_obj_remove_style(obj, &m_style, selector);
    }
}

void LVStyle::reset()
{
    if (m_initialized) {
        lv_style_reset(&m_style);
        lv_style_init(&m_style);
    }
}
