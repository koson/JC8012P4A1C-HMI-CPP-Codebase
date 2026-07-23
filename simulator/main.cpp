#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "hal/hal.h"
#include "lvgl.h"

#include "DrawTarget.hpp"
#include "JsonRenderer.hpp"
#include "LVCanvas.hpp"
#include "ElectricalPanel.h"
#include "font_thai.h"
#include "LessonPlayer.h"

static lv_obj_t *main_screen = NULL;
static lv_obj_t *demo_screen = NULL;

// Sample JSON circuit worksheet with Gates, Wires, Ports, and Junctions
static const char *sample_json = R"json({
  "version": "1.0",
  "title": "2-Stage AND Gate Circuit Demo",
  "description": "coord_mode=raw",
  "width": 1280,
  "height": 720,
  "backgroundColor": "#1E1E2E",
  "embeddedSymbols": {
    "AND_GATE_TEST": {
      "id": "AND_GATE_TEST",
      "title": "And Gate",
      "category": "Logic",
      "pathData": "M 140 30 L 160 30 M 60 15 L 80 15 M 60 45 L 80 45 M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 C 140 46.57 126.57 60 110 60 L 80 60 Z",
      "viewBox": {
        "x": 0,
        "y": 0,
        "width": 100,
        "height": 60
      }
    }
  },
  "widgets": [
    {
      "id": "title_label",
      "type": "label",
      "text": "LabBuddy - Logic Gate Circuit Simulation",
      "x": 350,
      "y": 40,
      "fontSize": 24,
      "textColor": "#00FF88",
      "fontWeight": "bold"
    },
    {
      "id": "gate1",
      "type": "svgSymbol",
      "x": 250,
      "y": 200,
      "width": 200,
      "height": 120,
      "rotation": 0,
      "symbolId": "AND_GATE_TEST",
      "scale": 2.0,
      "stroke": "#00FF88",
      "strokeWidth": 3
    },
    {
      "id": "gate2",
      "type": "svgSymbol",
      "x": 700,
      "y": 300,
      "width": 200,
      "height": 120,
      "rotation": 0,
      "symbolId": "AND_GATE_TEST",
      "scale": 2.0,
      "stroke": "#00E5FF",
      "strokeWidth": 3
    }
  ],
  "ports": [
    {
      "id": "portA",
      "label": "Input A",
      "type": "input",
      "x": 100,
      "y": 230,
      "radius": 10,
      "color": "#00FF88"
    },
    {
      "id": "portB",
      "label": "Input B",
      "type": "input",
      "x": 100,
      "y": 290,
      "radius": 10,
      "color": "#00FF88"
    },
    {
      "id": "portC",
      "label": "Input C",
      "type": "input",
      "x": 100,
      "y": 390,
      "radius": 10,
      "color": "#00E5FF"
    },
    {
      "id": "portOut",
      "label": "Output Y",
      "type": "output",
      "x": 1100,
      "y": 360,
      "radius": 12,
      "color": "#FFD700"
    }
  ],
  "wires": [
    {
      "id": "wireA",
      "path": "M 100 230 L 250 230",
      "color": "#00FF88",
      "strokeWidth": 3
    },
    {
      "id": "wireB",
      "path": "M 100 290 L 250 290",
      "color": "#00FF88",
      "strokeWidth": 3
    },
    {
      "id": "wireG1_to_G2",
      "path": "M 450 260 L 580 260 L 580 330 L 700 330",
      "color": "#00FF88",
      "strokeWidth": 3
    },
    {
      "id": "wireC",
      "path": "M 100 390 L 700 390",
      "color": "#00E5FF",
      "strokeWidth": 3
    },
    {
      "id": "wireOut",
      "path": "M 900 360 L 1100 360",
      "color": "#FFD700",
      "strokeWidth": 4
    }
  ],
  "junctions": [
    {
      "id": "j1",
      "x": 580,
      "y": 260,
      "radius": 5
    }
  ]
})json";

static void show_main_menu(void);

static void add_back_button(lv_obj_t *parent) {
  lv_obj_t *back_btn = lv_btn_create(parent);
  lv_obj_set_size(back_btn, 120, 50);
  lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 15, 15);
  lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xE74C3C), 0);
  lv_obj_move_foreground(back_btn);

  lv_obj_t *back_label = lv_label_create(back_btn);
  lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
  lv_obj_center(back_label);

  lv_obj_add_event_cb(
      back_btn,
      [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
          show_main_menu();
        }
      },
      LV_EVENT_CLICKED, NULL);
}

// Demo 1: Circuit Simulator (JSON Renderer)
static void run_circuit_demo(void) {
  demo_screen = lv_obj_create(NULL);
  lv_scr_load(demo_screen);
  lv_obj_set_style_bg_color(demo_screen, lv_color_hex(0x1E1E2E), 0);

  static uint8_t canvas_buf[1280 * 720 * 4];
  LVCanvas *canvas = new LVCanvas(demo_screen, 1280, 720, LV_COLOR_FORMAT_ARGB8888, canvas_buf);
  lv_obj_center(canvas->obj());

  JsonRenderer::JsonRenderer renderer(canvas);
  renderer.renderJsonString(sample_json);

  add_back_button(demo_screen);
}

// Demo 2: Electrical Panel
static void run_electrical_panel_demo(void) {
  demo_screen = lv_obj_create(NULL);
  lv_scr_load(demo_screen);
  lv_obj_set_style_bg_color(demo_screen, lv_color_hex(0x1A1A1A), 0);

  ElectricalPanel *panel = new ElectricalPanel(demo_screen);
  panel->setPosition(100, 80);
  panel->updateAll(220.5f, 221.0f, 219.8f, 5.2f, 4.8f, 5.5f, 1146.6f, 1060.8f,
                   1208.9f, 120.0f, 110.0f, 130.0f, 12.5f, 11.8f, 13.2f);

  add_back_button(demo_screen);
}

// Demo 3: Widget Gallery
static void run_widget_gallery_demo(void) {
  demo_screen = lv_obj_create(NULL);
  lv_scr_load(demo_screen);
  lv_obj_set_style_bg_color(demo_screen, lv_color_hex(0x1E1E2E), 0);

  lv_obj_t *title = lv_label_create(demo_screen);
  lv_label_set_text(title, "LVGL Widget Gallery");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FF88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Slider
  lv_obj_t *slider_label = lv_label_create(demo_screen);
  lv_label_set_text(slider_label, "Brightness Slider:");
  lv_obj_set_style_text_color(slider_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(slider_label, LV_ALIGN_TOP_LEFT, 200, 120);

  lv_obj_t *slider = lv_slider_create(demo_screen);
  lv_obj_set_width(slider, 400);
  lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 380, 115);
  lv_slider_set_value(slider, 70, LV_ANIM_OFF);

  // Switch
  lv_obj_t *sw_label = lv_label_create(demo_screen);
  lv_label_set_text(sw_label, "Power Switch:");
  lv_obj_set_style_text_color(sw_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(sw_label, LV_ALIGN_TOP_LEFT, 200, 190);

  lv_obj_t *sw = lv_switch_create(demo_screen);
  lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 380, 185);
  lv_obj_add_state(sw, LV_STATE_CHECKED);

  // Arc Gauge
  lv_obj_t *arc = lv_arc_create(demo_screen);
  lv_obj_set_size(arc, 200, 200);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_value(arc, 65);
  lv_obj_align(arc, LV_ALIGN_TOP_LEFT, 380, 260);

  add_back_button(demo_screen);
}

// Demo 4: Thai Font & Label Demo
static void run_thai_font_demo(void) {
  demo_screen = lv_obj_create(NULL);
  lv_scr_load(demo_screen);
  lv_obj_set_style_bg_color(demo_screen, lv_color_hex(0x16213E), 0);

  lv_obj_t *title = lv_label_create(demo_screen);
  lv_label_set_text(title, "ทดสอบฟอนต์ภาษาไทย (Thai Font Test)");
  lv_obj_set_style_text_color(title, lv_color_hex(0x00E5FF), 0);
  lv_obj_set_style_text_font(title, &th_niramit_32, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

  lv_obj_t *msg1 = lv_label_create(demo_screen);
  lv_label_set_text(msg1, "ยินดีต้อนรับสู่ระบบ LabBuddy-JC8012P4 HMI");
  lv_obj_set_style_text_color(msg1, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(msg1, &th_niramit_24, 0);
  lv_obj_align(msg1, LV_ALIGN_TOP_LEFT, 150, 150);

  lv_obj_t *msg2 = lv_label_create(demo_screen);
  lv_label_set_text(msg2, "ระบบจำลองการทำงานบน PC (Windows Simulator)");
  lv_obj_set_style_text_color(msg2, lv_color_hex(0x00FF88), 0);
  lv_obj_set_style_text_font(msg2, &th_niramit_20, 0);
  lv_obj_align(msg2, LV_ALIGN_TOP_LEFT, 150, 220);

  add_back_button(demo_screen);
}

// Demo 5: Interactive Lesson Player (L001 NOT Gate)
static void run_lesson_player_demo(void) {
  LessonPlayer &lp = LessonPlayer::getInstance();
  if (lp.loadLesson("simulator/worksheets/L001_not_gate.json")) {
    lp.show();
  } else {
    printf("[PC Simulator] Failed to load lesson file simulator/worksheets/L001_not_gate.json\n");
  }
}

// Main Menu Screen
static void show_main_menu(void) {
  if (main_screen) {
    lv_obj_del(main_screen);
  }
  main_screen = lv_obj_create(NULL);
  lv_scr_load(main_screen);
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x1A1A2E), 0);

  // Title
  lv_obj_t *title = lv_label_create(main_screen);
  lv_label_set_text(title, "LabBuddy HMI - Main Menu");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FF88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Subtitle
  lv_obj_t *sub = lv_label_create(main_screen);
  lv_label_set_text(sub, "Select a system module or interactive lesson to run on PC Simulator");
  lv_obj_set_style_text_color(sub, lv_color_hex(0xAAAAAA), 0);
  lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 60);

  // Container for Menu Buttons
  lv_obj_t *container = lv_obj_create(main_screen);
  lv_obj_set_size(container, 950, 560);
  lv_obj_align(container, LV_ALIGN_CENTER, 0, 45);
  lv_obj_set_style_bg_color(container, lv_color_hex(0x16213E), 0);
  lv_obj_set_style_border_color(container, lv_color_hex(0x0F4C75), 0);
  lv_obj_set_style_border_width(container, 2, 0);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(container, 15, 0);
  lv_obj_set_style_pad_row(container, 12, 0);

  struct MenuOption {
    const char *name;
    const char *desc;
    void (*func)(void);
    uint32_t color;
  };

  MenuOption options[] = {
      {"1. Interactive Lesson Player (L001 NOT Gate)",
       "Multi-page course engine (Cover -> Theory -> Interactive Circuit -> Verify)", run_lesson_player_demo,
       0x0F4C75},
      {"2. Logic Circuit Simulator (JSON Renderer)",
       "Render Gates, Wires, Ports & Junctions from JSON", run_circuit_demo,
       0x0F4C75},
      {"3. 3-Phase Electrical Parameters Panel",
       "Monitor Voltage, Current, Power & Energy", run_electrical_panel_demo,
       0x0F4C75},
      {"4. HMI Widget Gallery & Controls",
       "Test Sliders, Switches, Gauges & Controls", run_widget_gallery_demo,
       0x0F4C75},
      {"5. Thai Language & Font Rendering Test",
       "Display Thai fonts (Niramit 20/24/32)", run_thai_font_demo, 0x0F4C75},
  };

  for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
    lv_obj_t *btn = lv_btn_create(container);
    lv_obj_set_width(btn, lv_pct(100));
    lv_obj_set_height(btn, 85);
    lv_obj_set_style_bg_color(btn, lv_color_hex(options[i].color), 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3DBDE6), LV_STATE_PRESSED);

    lv_obj_t *lbl_title = lv_label_create(btn);
    lv_label_set_text(lbl_title, options[i].name);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_LEFT, 15, 10);

    lv_obj_t *lbl_desc = lv_label_create(btn);
    lv_label_set_text(lbl_desc, options[i].desc);
    lv_obj_set_style_text_color(lbl_desc, lv_color_hex(0xBDC3C7), 0);
    lv_obj_align(lbl_desc, LV_ALIGN_TOP_LEFT, 15, 45);

    void (*target_func)(void) = options[i].func;
    lv_obj_add_event_cb(
        btn,
        [](lv_event_t *e) {
          if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
            auto fn = (void (*)(void))lv_event_get_user_data(e);
            if (fn) fn();
          }
        },
        LV_EVENT_CLICKED, (void *)target_func);
  }
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  printf("[PC Simulator] Initializing LVGL 9...\n");
  lv_init();

  printf("[PC Simulator] Initializing SDL Display HAL (1280x800)...\n");
  sdl_hal_init(1280, 800);

  printf("[PC Simulator] Loading Main Menu...\n");
  show_main_menu();

  printf("[PC Simulator] Running main loop...\n");
  while (1) {
    uint32_t sleep_time_ms = lv_timer_handler();
    if (sleep_time_ms == LV_NO_TIMER_READY) {
      sleep_time_ms = LV_DEF_REFR_PERIOD;
    }
#ifdef _WIN32
    Sleep(sleep_time_ms);
#else
    usleep(sleep_time_ms * 1000);
#endif
  }

  return 0;
}
