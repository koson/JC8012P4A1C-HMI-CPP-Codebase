#include "lvgl.h"
#include "LVDisplay.hpp"
#include <array>
#include <cstdio>
#include <tuple>
#include "esp_lcd_touch.h"

// Mirror of lvgl_port_touch_ctx_t from esp_lvgl_port_touch.c
struct PortTouchCtx {
    esp_lcd_touch_handle_t handle;
    lv_indev_t* indev;
    struct { float x; float y; } scale;
};

// Basic correction flags (tune if coordinates don't align with finger)
// Landscape default: no swap, no mirror. Toggle if axes are flipped.
static constexpr bool TOUCH_SWAP_XY = false;
static constexpr bool TOUCH_MIRROR_X = false;
static constexpr bool TOUCH_MIRROR_Y = false;

static void apply_transform(lv_display_t* disp, lv_point_t& p)
{
    if (!disp) return;
    lv_display_rotation_t rot = lv_display_get_rotation(disp);
    int32_t hres = lv_display_get_horizontal_resolution(disp);
    int32_t vres = lv_display_get_vertical_resolution(disp);

    // Optional hardware correction before considering display rotation
    if (TOUCH_SWAP_XY) {
        std::swap(p.x, p.y);
        std::swap(hres, vres);
    }
    if (TOUCH_MIRROR_X) {
        p.x = hres - 1 - p.x;
    }
    if (TOUCH_MIRROR_Y) {
        p.y = vres - 1 - p.y;
    }

    // Adjust for display rotation
    int32_t x = p.x, y = p.y;
    switch (rot) {
        case LV_DISPLAY_ROTATION_0: break;
        case LV_DISPLAY_ROTATION_90: p.x = y; p.y = hres - 1 - x; break;
        case LV_DISPLAY_ROTATION_180: p.x = hres - 1 - x; p.y = vres - 1 - y; break;
        case LV_DISPLAY_ROTATION_270: p.x = vres - 1 - y; p.y = x; break;
    }
}

extern "C" void test_multitouch_demo()
{
    LVDisplay& disp = LVDisplay::getDefault();
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0B1118), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(scr, 10, 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(scr, 6, 0);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Multi-touch Monitor (GT911)");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    lv_obj_t* info = lv_label_create(scr);
    lv_obj_set_style_text_color(info, lv_color_hex(0xA0D8FF), 0);
    lv_label_set_text(info, "Reading touch points...");

    lv_obj_t* layer = lv_obj_create(scr);
    lv_obj_set_size(layer, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(layer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(layer, 0, 0);
    lv_obj_set_style_pad_all(layer, 0, 0);

    std::array<lv_obj_t*, CONFIG_ESP_LCD_TOUCH_MAX_POINTS> dots{};
    for (auto& d : dots) {
        d = lv_obj_create(layer);
        lv_obj_set_size(d, 20, 20);
        lv_obj_remove_flag(d, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(d, lv_palette_main(LV_PALETTE_ORANGE), 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(d, 0, 0);
        lv_obj_add_flag(d, LV_OBJ_FLAG_HIDDEN);
    }

    lv_indev_t* indev = lv_indev_get_next(nullptr);
    if (!indev) {
        lv_label_set_text(info, "No input device found");
        disp.setActiveScreen(scr);
        return;
    }

    PortTouchCtx* ctx = static_cast<PortTouchCtx*>(lv_indev_get_driver_data(indev));
    if (!ctx || !ctx->handle) {
        lv_label_set_text(info, "Touch handle missing");
        disp.setActiveScreen(scr);
        return;
    }

    static lv_timer_t* timer = nullptr;
    timer = lv_timer_create([](lv_timer_t* t) {
        auto data = static_cast<std::tuple<PortTouchCtx*, lv_obj_t*, std::array<lv_obj_t*, CONFIG_ESP_LCD_TOUCH_MAX_POINTS>, lv_display_t*>*>(lv_timer_get_user_data(t));
        auto [ctx_local, info_local, dots_local, disp_local] = *data;

        esp_lcd_touch_point_data_t points[CONFIG_ESP_LCD_TOUCH_MAX_POINTS] = {};
        uint8_t count = 0;
        esp_lcd_touch_read_data(ctx_local->handle);
        esp_lcd_touch_get_data(ctx_local->handle, points, &count, CONFIG_ESP_LCD_TOUCH_MAX_POINTS);

        // Hide all first
        for (auto* d : dots_local) {
            lv_obj_add_flag(d, LV_OBJ_FLAG_HIDDEN);
        }

        char buf[160];
        int len = std::snprintf(buf, sizeof(buf), "Points: %u\n", count);

        for (uint8_t i = 0; i < count && i < dots_local.size(); ++i) {
            lv_point_t p { (lv_coord_t)(ctx_local->scale.x * points[i].x), (lv_coord_t)(ctx_local->scale.y * points[i].y) };
            apply_transform(disp_local, p);
            lv_obj_clear_flag(dots_local[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(dots_local[i], p.x - 10, p.y - 10);
            len += std::snprintf(buf + len, sizeof(buf) - len, "[%u] %d,%d id=%u\n", i, p.x, p.y, points[i].track_id);
        }
        lv_label_set_text(info_local, buf);
    }, 60, nullptr);

    // store data for timer
    auto* payload = new std::tuple<PortTouchCtx*, lv_obj_t*, std::array<lv_obj_t*, CONFIG_ESP_LCD_TOUCH_MAX_POINTS>, lv_display_t*>(ctx, info, dots, disp.raw());
    lv_timer_set_user_data(timer, payload);

    disp.setActiveScreen(scr);
}
