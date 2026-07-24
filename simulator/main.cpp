#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "hal/hal.h"
#include "lvgl.h"

#include "DrawTarget.hpp"
#include "ElectricalPanel.h"
#include "JsonRenderer.hpp"
#include "LVCanvas.hpp"
#include "LVScreen.hpp"
#include "ScreenManager.hpp"
#include "LessonPlayer.h"
#include "font_thai.h"

static void show_main_menu(void);
static void run_lesson_browser_demo(void);

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
          ScreenManager::getInstance().back(LVScreen::Transition::MoveRight, 200);
        }
      },
      LV_EVENT_CLICKED, NULL);
}

// Function to open any selected lesson JSON file
static void open_worksheet_file(const std::string &full_path) {
  printf("[LessonBrowser] Opening worksheet: %s\n", full_path.c_str());

  FILE *f = fopen(full_path.c_str(), "r");
  if (!f) {
    printf("[LessonBrowser] Cannot open file: %s\n", full_path.c_str());
    return;
  }
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);

  bool is_multipage_lesson = false;
  if (sz > 0) {
    char *buf = (char *)malloc(sz + 1);
    if (buf) {
      size_t n = fread(buf, 1, sz, f);
      buf[n] = '\0';
      cJSON *root = cJSON_Parse(buf);
      free(buf);
      if (root) {
        cJSON *pages = cJSON_GetObjectItem(root, "pages");
        if (pages && cJSON_IsArray(pages)) {
          is_multipage_lesson = true;
        }
        cJSON_Delete(root);
      }
    }
  }
  fclose(f);

  if (is_multipage_lesson) {
    LessonPlayer &lp = LessonPlayer::getInstance();
    if (lp.loadLesson(full_path.c_str())) {
      lp.show();
      return;
    }
  }

  // Render single-page JSON worksheet using ScreenManager
  LVScreen *scr = new LVScreen();
  lv_obj_t *root = scr->getObj();
  lv_obj_set_style_bg_color(root, lv_color_hex(0x1E1E2E), 0);

  static uint8_t canvas_buf[1280 * 720 * 4];
  LVCanvas *canvas = new LVCanvas(root, 1280, 720, LV_COLOR_FORMAT_ARGB8888, canvas_buf);
  lv_obj_center(canvas->obj());

  JsonRenderer::JsonRenderer renderer(canvas);
  if (!renderer.loadAndRender(full_path.c_str())) {
    printf("[LessonBrowser] Failed to render circuit: %s\n", renderer.getLastError());
  }

  add_back_button(root);

  ScreenManager &mgr = ScreenManager::getInstance();
  mgr.registerScreen("circuit_viewer", scr);
  mgr.push("circuit_viewer", LVScreen::Transition::MoveLeft, 200);
}

// Demo: Lesson & Worksheet Browser
static void run_lesson_browser_demo(void) {
  LVScreen *scr = new LVScreen();
  lv_obj_t *root = scr->getObj();
  lv_obj_set_style_bg_color(root, lv_color_hex(0x1A1A2E), 0);

  // Title
  lv_obj_t *title = lv_label_create(root);
  lv_label_set_text(title, "Lesson & Worksheet File Browser");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FF88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  add_back_button(root);

  // Scan worksheets folder
  std::vector<std::string> json_files;
  const char *folder_paths[] = {"worksheets", "simulator/worksheets", "../simulator/worksheets"};
  std::string active_folder = "";

  for (const char *fp : folder_paths) {
    DIR *dir = opendir(fp);
    if (dir) {
      active_folder = fp;
      struct dirent *ent;
      while ((ent = readdir(dir)) != NULL) {
        std::string fname = ent->d_name;
        if (fname.length() > 5 && fname.substr(fname.length() - 5) == ".json") {
          json_files.push_back(fname);
        }
      }
      closedir(dir);
      if (!json_files.empty()) break;
    }
  }

  std::sort(json_files.begin(), json_files.end());

  // Scrollable list container
  lv_obj_t *container = lv_obj_create(root);
  lv_obj_set_size(container, 1000, 580);
  lv_obj_align(container, LV_ALIGN_CENTER, 0, 35);
  lv_obj_set_style_bg_color(container, lv_color_hex(0x16213E), 0);
  lv_obj_set_style_border_color(container, lv_color_hex(0x0F4C75), 0);
  lv_obj_set_style_border_width(container, 2, 0);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(container, 15, 0);
  lv_obj_set_style_pad_row(container, 10, 0);

  if (json_files.empty()) {
    lv_obj_t *lbl_empty = lv_label_create(container);
    lv_label_set_text(lbl_empty, "No .json worksheet files found in worksheets directory.");
    lv_obj_set_style_text_color(lbl_empty, lv_color_hex(0xE74C3C), 0);
  } else {
    for (size_t i = 0; i < json_files.size(); i++) {
      lv_obj_t *btn = lv_btn_create(container);
      lv_obj_set_width(btn, lv_pct(100));
      lv_obj_set_height(btn, 65);
      lv_obj_set_style_bg_color(btn, lv_color_hex(0x0F4C75), 0);
      lv_obj_set_style_bg_color(btn, lv_color_hex(0x3DBDE6), LV_STATE_PRESSED);

      std::string full_path = active_folder + "/" + json_files[i];

      lv_obj_t *lbl = lv_label_create(btn);
      lv_label_set_text_fmt(lbl, LV_SYMBOL_FILE " %d. %s", (int)(i + 1), json_files[i].c_str());
      lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
      lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
      lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 15, 0);

      char *path_copy = strdup(full_path.c_str());
      lv_obj_add_event_cb(
          btn,
          [](lv_event_t *e) {
            if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
              char *p = (char *)lv_event_get_user_data(e);
              if (p) {
                open_worksheet_file(std::string(p));
              }
            }
          },
          LV_EVENT_CLICKED, (void *)path_copy);
    }
  }

  ScreenManager &mgr = ScreenManager::getInstance();
  mgr.registerScreen("lesson_browser", scr);
  mgr.push("lesson_browser", LVScreen::Transition::MoveLeft, 200);
}

// Demo 3: Logic Circuit Simulator (2-Stage AND Circuit)
static void run_circuit_demo(void) {
  const char *paths[] = {"worksheets/and2_circuit.json", "simulator/worksheets/and2_circuit.json"};
  for (const char *p : paths) {
    FILE *f = fopen(p, "r");
    if (f) {
      fclose(f);
      open_worksheet_file(p);
      return;
    }
  }
}

// Demo 2: Electrical Panel
static void run_electrical_panel_demo(void) {
  LVScreen *scr = new LVScreen();
  lv_obj_t *root = scr->getObj();
  lv_obj_set_style_bg_color(root, lv_color_hex(0x1A1A1A), 0);

  ElectricalPanel *panel = new ElectricalPanel(root);
  panel->setPosition(100, 80);
  panel->updateAll(220.5f, 221.0f, 219.8f, 5.2f, 4.8f, 5.5f, 1146.6f, 1060.8f,
                   1208.9f, 120.0f, 110.0f, 130.0f, 12.5f, 11.8f, 13.2f);

  add_back_button(root);

  ScreenManager &mgr = ScreenManager::getInstance();
  mgr.registerScreen("electrical_panel", scr);
  mgr.push("electrical_panel", LVScreen::Transition::MoveLeft, 200);
}

// Demo 3: Widget Gallery
static void run_widget_gallery_demo(void) {
  LVScreen *scr = new LVScreen();
  lv_obj_t *root = scr->getObj();
  lv_obj_set_style_bg_color(root, lv_color_hex(0x1E1E2E), 0);

  lv_obj_t *title = lv_label_create(root);
  lv_label_set_text(title, "LVGL Widget Gallery");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FF88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Slider
  lv_obj_t *slider_label = lv_label_create(root);
  lv_label_set_text(slider_label, "Brightness Slider:");
  lv_obj_set_style_text_color(slider_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(slider_label, LV_ALIGN_TOP_LEFT, 200, 120);

  lv_obj_t *slider = lv_slider_create(root);
  lv_obj_set_width(slider, 400);
  lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 380, 115);
  lv_slider_set_value(slider, 70, LV_ANIM_OFF);

  // Switch
  lv_obj_t *sw_label = lv_label_create(root);
  lv_label_set_text(sw_label, "Power Switch:");
  lv_obj_set_style_text_color(sw_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(sw_label, LV_ALIGN_TOP_LEFT, 200, 190);

  lv_obj_t *sw = lv_switch_create(root);
  lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 380, 185);
  lv_obj_add_state(sw, LV_STATE_CHECKED);

  // Arc Gauge
  lv_obj_t *arc = lv_arc_create(root);
  lv_obj_set_size(arc, 200, 200);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_value(arc, 65);
  lv_obj_align(arc, LV_ALIGN_TOP_LEFT, 380, 260);

  add_back_button(root);

  ScreenManager &mgr = ScreenManager::getInstance();
  mgr.registerScreen("widget_gallery", scr);
  mgr.push("widget_gallery", LVScreen::Transition::MoveLeft, 200);
}

// Demo 4: Thai Font & Label Demo
static void run_thai_font_demo(void) {
  LVScreen *scr = new LVScreen();
  lv_obj_t *root = scr->getObj();
  lv_obj_set_style_bg_color(root, lv_color_hex(0x16213E), 0);

  lv_obj_t *title = lv_label_create(root);
  lv_label_set_text(title, "ทดสอบฟอนต์ภาษาไทย (Thai Font Test)");
  lv_obj_set_style_text_color(title, lv_color_hex(0x00E5FF), 0);
  lv_obj_set_style_text_font(title, &th_niramit_32, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

  lv_obj_t *msg1 = lv_label_create(root);
  lv_label_set_text(msg1, "ยินดีต้อนรับสู่ระบบ LabBuddy-JC8012P4 HMI");
  lv_obj_set_style_text_color(msg1, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(msg1, &th_niramit_24, 0);
  lv_obj_align(msg1, LV_ALIGN_TOP_LEFT, 150, 150);

  lv_obj_t *msg2 = lv_label_create(root);
  lv_label_set_text(msg2, "ระบบจำลองการทำงานบน PC (Windows Simulator)");
  lv_obj_set_style_text_color(msg2, lv_color_hex(0x00FF88), 0);
  lv_obj_set_style_text_font(msg2, &th_niramit_20, 0);
  lv_obj_align(msg2, LV_ALIGN_TOP_LEFT, 150, 220);

  add_back_button(root);

  ScreenManager &mgr = ScreenManager::getInstance();
  mgr.registerScreen("thai_font", scr);
  mgr.push("thai_font", LVScreen::Transition::MoveLeft, 200);
}

// Demo 5: Interactive Lesson Player (L001 NOT Gate)
static void run_lesson_player_demo(void) {
  LessonPlayer &lp = LessonPlayer::getInstance();
  const char *paths[] = {"worksheets/Not_Gate_Lesson_Circuit.json",
                         "worksheets/L001_not_gate.json",
                         "simulator/worksheets/L001_not_gate.json",
                         "../simulator/worksheets/L001_not_gate.json"};
  bool loaded = false;
  for (const char *p : paths) {
    if (lp.loadLesson(p)) {
      loaded = true;
      break;
    }
  }
  if (loaded) {
    lp.show();
  } else {
    printf("[PC Simulator] Failed to load lesson file L001_not_gate.json\n");
  }
}

// Main Menu Screen
static void show_main_menu(void) {
  LVScreen *scr = new LVScreen();
  lv_obj_t *root = scr->getObj();
  lv_obj_set_style_bg_color(root, lv_color_hex(0x1A1A2E), 0);

  // Title
  lv_obj_t *title = lv_label_create(root);
  lv_label_set_text(title, "LabBuddy HMI - Main Menu");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FF88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  // Subtitle
  lv_obj_t *sub = lv_label_create(root);
  lv_label_set_text(
      sub,
      "Select a system module or interactive lesson to run on PC Simulator");
  lv_obj_set_style_text_color(sub, lv_color_hex(0xAAAAAA), 0);
  lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 60);

  // Container for Menu Buttons
  lv_obj_t *container = lv_obj_create(root);
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
      {"1. 📁 Lesson & Worksheet File Browser (Select & Run)",
       "Browse and render all 29+ JSON lessons & circuit worksheets", run_lesson_browser_demo,
       0x00A86B},
      {"2. Interactive Lesson Player (L001 NOT Gate)",
       "Multi-page course engine (Cover -> Theory -> Interactive Circuit -> Verify)",
       run_lesson_player_demo, 0x0F4C75},
      {"3. Logic Circuit Simulator (2-Stage AND Circuit)",
       "Render Gates, Wires, Ports & Junctions from JSON", run_circuit_demo,
       0x0F4C75},
      {"4. 3-Phase Electrical Parameters Panel",
       "Monitor Voltage, Current, Power & Energy", run_electrical_panel_demo,
       0x0F4C75},
      {"5. HMI Widget Gallery & Controls",
       "Test Sliders, Switches, Gauges & Controls", run_widget_gallery_demo,
       0x0F4C75},
      {"6. Thai Language & Font Rendering Test",
       "Display Thai fonts (Niramit 20/24/32)", run_thai_font_demo, 0x0F4C75},
  };

  for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
    lv_obj_t *btn = lv_btn_create(container);
    lv_obj_set_width(btn, lv_pct(100));
    lv_obj_set_height(btn, 80);
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
            if (fn)
              fn();
          }
        },
        LV_EVENT_CLICKED, (void *)target_func);
  }

  ScreenManager &mgr = ScreenManager::getInstance();
  mgr.registerScreen("main_menu", scr);
  mgr.navigateTo("main_menu", LVScreen::Transition::None, 0);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  printf("[PC Simulator] Initializing LVGL 9...\n");
  fflush(stdout);
  lv_init();

  printf("[PC Simulator] Initializing SDL Display HAL (1280x800)...\n");
  fflush(stdout);
  sdl_hal_init(1280, 800);

  printf("[PC Simulator] Loading Main Menu via ScreenManager...\n");
  fflush(stdout);
  show_main_menu();

  printf("[PC Simulator] Running main loop...\n");
  fflush(stdout);
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
