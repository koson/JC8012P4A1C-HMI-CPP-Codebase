#include "lvgl.h"
#include "LVColor.hpp"
#include "LVDisplay.hpp"
#include <array>

static lv_obj_t* make_swatches(lv_obj_t* parent, const char* title, const std::array<LVColor, 4>& colors)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 6, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* label = lv_label_create(cont);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_pad_right(label, 8, 0);

    for (auto& c : colors) {
        lv_obj_t* box = lv_obj_create(cont);
        lv_obj_set_size(box, 60, 40);
        lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(box, 6, 0);
        lv_obj_set_style_bg_color(box, c.raw(), 0);
        lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(box, 0, 0);
    }

    return cont;
}

static void add_text(lv_obj_t* parent, const char* txt)
{
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(0xA0D8FF), 0);
}

extern "C" void test_color_demo()
{
    LVDisplay::getDefault().setActiveScreen(lv_scr_act());

    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0F141C), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(scr, 12, 0);
    lv_obj_set_style_pad_row(scr, 6, 0);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LVColor Demo");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    // Base colors
    make_swatches(scr, "Base:", {LVColor::Red, LVColor::Green, LVColor::Blue, LVColor::Yellow});

    // Lighten/Darken from a base
    LVColor base = LVColor(0x1565C0);
    make_swatches(scr, "Lighten/Darken:", {base.lighten(80), base, base.darken(80), LVColor::Gray});

    // Mix two colors 75/25
    LVColor a = LVColor(0xFF7043);
    LVColor b = LVColor(0x26C6DA);
    make_swatches(scr, "Mix 75/25:", {a, b, a.mix(b, 192), b.mix(a, 192)});

    // HSV helper
    LVColor hsv = LVColor::fromHSV(200, 80, 90);
    make_swatches(scr, "HSV(200,80,90):", {hsv, hsv.lighten(60), hsv.darken(60), LVColor::Black});

    // Hex string display
    add_text(scr, LVColor(0xFF7043).toHexString().c_str());
    add_text(scr, LVColor::fromHSV(320, 70, 90).toHexString().c_str());

    LVDisplay::getDefault().setActiveScreen(scr);
}
