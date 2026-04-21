#pragma once

#include "lvgl.h"
#include "LVColor.hpp"

/**
 * @brief LVGL Style wrapper class for centralized style management
 * 
 * Provides type-safe, object-oriented interface for creating and managing
 * LVGL styles. Supports all common styling properties with automatic
 * resource management.
 * 
 * @example
 * ```cpp
 * LVStyle panel_style;
 * panel_style.setBackgroundColor(LVColor(0x1E1E1E))
 *            .setBorderColor(LVColor(0x444444))
 *            .setBorderWidth(2)
 *            .setRadius(8)
 *            .setPadding(10);
 * panel_style.applyTo(my_panel);
 * ```
 */
class LVStyle {
public:
    /**
     * @brief Constructor - initializes LVGL style
     */
    LVStyle();
    
    /**
     * @brief Destructor - cleans up style resources
     */
    ~LVStyle();
    
    // Disable copy (styles should be unique)
    LVStyle(const LVStyle&) = delete;
    LVStyle& operator=(const LVStyle&) = delete;
    
    // Allow move
    LVStyle(LVStyle&& other) noexcept;
    LVStyle& operator=(LVStyle&& other) noexcept;
    
    // ========== Size & Layout ==========
    
    LVStyle& setWidth(int32_t width);
    LVStyle& setHeight(int32_t height);
    LVStyle& setMinWidth(int32_t width);
    LVStyle& setMinHeight(int32_t height);
    LVStyle& setMaxWidth(int32_t width);
    LVStyle& setMaxHeight(int32_t height);
    
    // Padding (internal spacing)
    LVStyle& setPadding(int32_t all);
    LVStyle& setPadding(int32_t top, int32_t right, int32_t bottom, int32_t left);
    LVStyle& setPaddingTop(int32_t value);
    LVStyle& setPaddingRight(int32_t value);
    LVStyle& setPaddingBottom(int32_t value);
    LVStyle& setPaddingLeft(int32_t value);
    
    // Margin (external spacing)
    LVStyle& setMargin(int32_t all);
    LVStyle& setMargin(int32_t top, int32_t right, int32_t bottom, int32_t left);
    
    // ========== Colors & Appearance ==========
    
    // Background
    LVStyle& setBackgroundColor(LVColor color);
    LVStyle& setBackgroundOpacity(lv_opa_t opa);
    LVStyle& setBackgroundGradientColor(LVColor color);
    LVStyle& setBackgroundGradientDirection(lv_grad_dir_t dir);
    
    // Border
    LVStyle& setBorderColor(LVColor color);
    LVStyle& setBorderWidth(int32_t width);
    LVStyle& setBorderOpacity(lv_opa_t opa);
    LVStyle& setBorderSide(lv_border_side_t side);  // TOP, BOTTOM, LEFT, RIGHT, FULL
    
    // Outline (border outside the widget)
    LVStyle& setOutlineColor(LVColor color);
    LVStyle& setOutlineWidth(int32_t width);
    LVStyle& setOutlinePad(int32_t pad);
    LVStyle& setOutlineOpacity(lv_opa_t opa);
    
    // Shadow
    LVStyle& setShadowColor(LVColor color);
    LVStyle& setShadowWidth(int32_t width);
    LVStyle& setShadowOffset(int32_t x, int32_t y);
    LVStyle& setShadowSpread(int32_t spread);
    LVStyle& setShadowOpacity(lv_opa_t opa);
    
    // Radius
    LVStyle& setRadius(int32_t radius);
    
    // Opacity
    LVStyle& setOpacity(lv_opa_t opa);
    
    // ========== Text Styling ==========
    
    LVStyle& setTextColor(LVColor color);
    LVStyle& setTextOpacity(lv_opa_t opa);
    LVStyle& setTextFont(const lv_font_t* font);
    LVStyle& setTextAlign(lv_text_align_t align);  // LEFT, CENTER, RIGHT
    LVStyle& setTextLetterSpacing(int32_t space);
    LVStyle& setTextLineSpacing(int32_t space);
    LVStyle& setTextDecoration(lv_text_decor_t decor);  // NONE, UNDERLINE, STRIKETHROUGH
    
    // ========== Image Styling ==========
    
    LVStyle& setImageOpacity(lv_opa_t opa);
    LVStyle& setImageRecolor(LVColor color);
    LVStyle& setImageRecolorOpacity(lv_opa_t opa);
    
    // ========== Line Styling ==========
    
    LVStyle& setLineColor(LVColor color);
    LVStyle& setLineWidth(int32_t width);
    LVStyle& setLineOpacity(lv_opa_t opa);
    LVStyle& setLineRounded(bool rounded);
    
    // ========== Arc Styling ==========
    
    LVStyle& setArcColor(LVColor color);
    LVStyle& setArcWidth(int32_t width);
    LVStyle& setArcOpacity(lv_opa_t opa);
    LVStyle& setArcRounded(bool rounded);
    
    // ========== Blend Mode & Transform ==========
    
    LVStyle& setBlendMode(lv_blend_mode_t mode);
    LVStyle& setTransformAngle(int32_t angle);  // 0.1 degree units
    LVStyle& setTransformZoom(int32_t zoom);    // 256 = 100%
    
    // ========== Transitions ==========
    
    LVStyle& setTransition(const lv_style_transition_dsc_t* trans);
    
    // ========== Application ==========
    
    /**
     * @brief Apply style to a widget
     * @param obj Widget to apply style to
     * @param selector Part and state selector (default: MAIN part, default state)
     */
    void applyTo(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /**
     * @brief Remove style from a widget
     * @param obj Widget to remove style from
     * @param selector Part and state selector
     */
    void removeFrom(lv_obj_t* obj, lv_style_selector_t selector = LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /**
     * @brief Get raw LVGL style pointer (for advanced usage)
     */
    lv_style_t* raw() { return &m_style; }
    const lv_style_t* raw() const { return &m_style; }
    
    /**
     * @brief Reset style to default state
     */
    void reset();
    
private:
    lv_style_t m_style;
    bool m_initialized;
};
