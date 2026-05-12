#pragma once
/**
 * ThaiLabel — LVGL object that renders Thai text with correct tone-mark placement.
 *
 * Uses the same Thai shaper as LVCanvas::drawTextThaiShaped().
 * 2-layer tone marks (กา̈น้) are extracted from the run and drawn at a
 * font-metric-derived offset so they never float above the label bounds.
 *
 * API mirrors lv_label where possible:
 *   lv_obj_t *lbl = thai_label_create(parent);
 *   thai_label_set_text(lbl, "ตั้งค่า");
 *   thai_label_set_font(lbl, th_niramit_select(40));
 *   thai_label_set_color(lbl, lv_color_hex(0xFFFFFF));
 *   lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, 12);  // normal LVGL positioning
 */
#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** Create a Thai-aware label as a child of @p parent.
     *  Background is transparent, no border, no padding by default.
     *  Size defaults to the parent's full content area; call lv_obj_set_size()
     *  or lv_obj_set_width() to constrain it.
     */
    lv_obj_t *thai_label_create(lv_obj_t *parent);

    /** Set the text to display (UTF-8, may contain Thai). */
    void thai_label_set_text(lv_obj_t *obj, const char *text);

    /** Select the font to use (e.g. th_niramit_select(32)). */
    void thai_label_set_font(lv_obj_t *obj, const lv_font_t *font);

    /** Set the text colour. */
    void thai_label_set_color(lv_obj_t *obj, lv_color_t color);

    /** Horizontal text alignment within the label area (default: CENTER). */
    void thai_label_set_align(lv_obj_t *obj, lv_text_align_t align);

#ifdef __cplusplus
}
#endif
