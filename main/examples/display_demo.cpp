#include "lvgl.h"
#include "LVDisplay.hpp"
#include <string>

static lv_obj_t* create_screen(lv_color_t bg, const char* title)
{
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, bg, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    return scr;
}

static void attach_controls(lv_obj_t* scr, lv_obj_t* other_scr, LVDisplay& disp)
{
    lv_obj_t* panel = lv_obj_create(scr);
    lv_obj_set_size(panel, lv_pct(100), 80);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_70, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x222222), 0);
    lv_obj_set_style_pad_all(panel, 10, 0);
    lv_obj_set_style_border_width(panel, 0, 0);

    lv_obj_t* info = lv_label_create(panel);
    char buf[128];
    lv_snprintf(buf, sizeof(buf), "Res: %ux%u dpi:%u", disp.horRes(), disp.verRes(), disp.dpi());
    lv_label_set_text(info, buf);
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_align(info, LV_ALIGN_LEFT_MID, 0, 0);

    // Rotation toggle
    lv_obj_t* btn_rot = lv_btn_create(panel);
    lv_obj_set_size(btn_rot, 140, 40);
    lv_obj_align(btn_rot, LV_ALIGN_CENTER, -80, 0);
    lv_obj_t* rot_label = lv_label_create(btn_rot);
    lv_label_set_text(rot_label, "Rotate 0/90");
    lv_obj_center(rot_label);

    lv_obj_add_event_cb(btn_rot, [](lv_event_t* e) {
        LVDisplay& d = LVDisplay::getDefault();
        auto current = d.getRotation();
        auto next = (current == LVDisplay::Rotation::Rotate0) ? LVDisplay::Rotation::Rotate90 : LVDisplay::Rotation::Rotate0;
        d.setRotation(next);
    }, LV_EVENT_CLICKED, nullptr);

    // Screen switch button
    lv_obj_t* btn_switch = lv_btn_create(panel);
    lv_obj_set_size(btn_switch, 140, 40);
    lv_obj_align(btn_switch, LV_ALIGN_CENTER, 80, 0);
    lv_obj_t* sw_label = lv_label_create(btn_switch);
    lv_label_set_text(sw_label, "Switch Screen");
    lv_obj_center(sw_label);

    lv_obj_add_event_cb(btn_switch, [](lv_event_t* e) {
        lv_obj_t* other = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        LVDisplay::getDefault().loadScreen(other, 300);
    }, LV_EVENT_CLICKED, other_scr);
}

extern "C" void test_display_demo()
{
    LVDisplay& display = LVDisplay::getDefault();

    lv_obj_t* screen1 = create_screen(lv_color_hex(0x1565C0), "Screen 1 - Info");
    lv_obj_t* screen2 = create_screen(lv_color_hex(0x6A1B9A), "Screen 2 - Metrics");

    attach_controls(screen1, screen2, display);
    attach_controls(screen2, screen1, display);

    display.setActiveScreen(screen1);
}
