#include "lvgl.h"
#include "LVCanvas.hpp"
#include "LVColor.hpp"
#include "LVDisplay.hpp"
#include "esp_heap_caps.h"
#include <cstring>
#include <array>

// Canvas dimensions (RGB565): ~96 KB for 240x200
static constexpr uint16_t CANVAS_W = 240;
static constexpr uint16_t CANVAS_H = 200;
static lv_color_t* s_canvas_buf = nullptr;
static LVCanvas* s_canvas_obj = nullptr;  // Keep canvas object alive

static void draw_scene(LVCanvas& canvas)
{
    canvas.clear(LVColor(0x0F141C));

    // Grid lines
    for (int x = 0; x < CANVAS_W; x += 24) {
        canvas.drawLine(x, 0, x, CANVAS_H - 1, LVColor(0x1E2A38), 1, LV_OPA_60);
    }
    for (int y = 0; y < CANVAS_H; y += 24) {
        canvas.drawLine(0, y, CANVAS_W - 1, y, LVColor(0x1E2A38), 1, LV_OPA_60);
    }

    // Colored blocks
    LVColor a(0xFF7043);
    LVColor b(0x26C6DA);
    canvas.drawRect(16, 16, 80, 60, a, LV_OPA_COVER, 8);
    canvas.drawRect(120, 16, 100, 60, b, LV_OPA_COVER, 12);
    canvas.drawRect(60, 96, 120, 70, a.mix(b, 160), LV_OPA_COVER, 16);

    // Text
    canvas.drawText(20, 172, "LVCanvas Demo", LVColor::White);
    canvas.drawText(150, 172, a.toHexString().c_str(), a.lighten(40));
}

extern "C" void test_canvas_demo()
{
    // Allocate canvas buffer - try internal memory first for cache coherency
    if (!s_canvas_buf) {
        // Try internal DMA-capable memory first (better for canvas operations)
        s_canvas_buf = (lv_color_t*)heap_caps_malloc(
            CANVAS_W * CANVAS_H * sizeof(lv_color_t), 
            MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL
        );
        
        if (!s_canvas_buf) {
            // Fallback to PSRAM
            s_canvas_buf = (lv_color_t*)heap_caps_malloc(
                CANVAS_W * CANVAS_H * sizeof(lv_color_t), 
                MALLOC_CAP_SPIRAM
            );
        }
        
        if (!s_canvas_buf) {
            // Last resort: default heap
            s_canvas_buf = (lv_color_t*)malloc(CANVAS_W * CANVAS_H * sizeof(lv_color_t));
        }
        
        // Verify allocation succeeded
        if (!s_canvas_buf) {
            LV_LOG_ERROR("Failed to allocate canvas buffer!");
            return;
        }
        
        // Zero-initialize buffer
        memset(s_canvas_buf, 0, CANVAS_W * CANVAS_H * sizeof(lv_color_t));
        
        LV_LOG_INFO("Canvas buffer allocated: %p (%d bytes)", s_canvas_buf, 
                    CANVAS_W * CANVAS_H * sizeof(lv_color_t));
    }

    LVDisplay& display = LVDisplay::getDefault();

    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0B1118), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(scr, 10, 0);
    lv_obj_set_style_pad_row(scr, 8, 0);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LVCanvas Demo (RGB565)");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    // Canvas host container
    lv_obj_t* canvas_holder = lv_obj_create(scr);
    lv_obj_set_size(canvas_holder, CANVAS_W + 4, CANVAS_H + 4);
    lv_obj_set_style_pad_all(canvas_holder, 2, 0);
    lv_obj_set_style_bg_color(canvas_holder, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(canvas_holder, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(canvas_holder, 0, 0);

    // Create canvas widget (heap allocated to persist beyond function scope)
    if (s_canvas_obj) {
        delete s_canvas_obj;
    }
    s_canvas_obj = new LVCanvas(canvas_holder, CANVAS_W, CANVAS_H, LV_COLOR_FORMAT_RGB565, s_canvas_buf);
    lv_obj_set_size(s_canvas_obj->obj(), CANVAS_W, CANVAS_H);

    draw_scene(*s_canvas_obj);

    // Buttons row
    lv_obj_t* row = lv_obj_create(scr);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 8, 0);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    auto make_btn = [&](const char* txt, lv_event_cb_t cb, void* ud) {
        lv_obj_t* btn = lv_btn_create(row);
        lv_obj_set_size(btn, 96, 36);
        lv_obj_t* l = lv_label_create(btn);
        lv_label_set_text(l, txt);
        lv_obj_center(l);
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, ud);
    };

    make_btn("Redraw", [](lv_event_t* e) {
        auto* c = static_cast<LVCanvas*>(lv_event_get_user_data(e));
        draw_scene(*c);
    }, s_canvas_obj);

    make_btn("Invert", [](lv_event_t* e) {
        auto* c = static_cast<LVCanvas*>(lv_event_get_user_data(e));
        // Simple invert by drawing overlay rectangles
        c->drawRect(0, 0, CANVAS_W, CANVAS_H, LVColor::White, LV_OPA_20);
    }, s_canvas_obj);

    display.setActiveScreen(scr);
}
