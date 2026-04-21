#include "lvgl.h"
#include "LVIndev.hpp"
#include "LVDisplay.hpp"
#include "LVTimer.hpp"
#include <cstdio>

extern "C" void test_indev_demo()
{
    LVDisplay::getDefault().setActiveScreen(lv_scr_act());

    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LVIndev Demo - Touch Monitor");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, "Waiting for input device...");
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 8, 40);

    // Create a small cursor dot and attach to pointer device
    lv_obj_t* cursor = lv_label_create(scr);
    lv_label_set_text(cursor, "•");
    lv_obj_set_style_text_color(cursor, lv_palette_main(LV_PALETTE_CYAN), 0);
    lv_obj_set_style_text_font(cursor, &lv_font_montserrat_20, 0);

    LVIndev indev = LVIndev::getDefault();
    if (!indev.valid()) {
        lv_label_set_text(info, "No input device found");
        LVDisplay::getDefault().setActiveScreen(scr);
        return;
    }

    indev.setCursor(cursor);
    indev.enableRotationTransform(LVDisplay::getDefault().raw());

    // Periodically poll the pointer state and update label
    static LVTimer pollTimer(50, [info, indev]() mutable {
        lv_point_t pt {};
        bool has_point = indev.getPoint(pt);
        auto state = indev.getState();

        char buf[96];
        if (has_point) {
            lv_snprintf(buf, sizeof(buf), "Type: %d  State: %s\nPoint: (%d,%d)",
                        static_cast<int>(indev.type()),
                        state == LV_INDEV_STATE_PRESSED ? "PRESSED" : "RELEASED",
                        pt.x, pt.y);
        } else {
            lv_snprintf(buf, sizeof(buf), "Type: %d  State: %s\nPoint: (n/a)",
                        static_cast<int>(indev.type()),
                        state == LV_INDEV_STATE_PRESSED ? "PRESSED" : "RELEASED");
        }
        lv_label_set_text(info, buf);
    });

    // Simple hint box at bottom
    lv_obj_t* hint = lv_label_create(scr);
    lv_label_set_text(hint, "Tap/drag to see coords. Cursor follows touch.");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xA0D8FF), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -12);

    LVDisplay::getDefault().setActiveScreen(scr);
}
