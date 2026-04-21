#include "main_menu.h"
#include "lvgl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "MAIN_MENU";

// External demo functions
extern "C" {
    // Phase 1
    void test_cpp_example(void);
    
    // Phase 2: Widgets
    void test_brightness_control(void);
    void test_switch_control(void);
    void test_button_control(void);
    void test_checkbox_control(void);
    void test_dropdown_selection(void);
    void test_textarea_editor(void);
    void test_arc_control(void);
    void test_bar_progress(void);
    void test_chart_demo(void);
    void test_roller_selection(void);
    void test_image_display(void);
    void test_spinner_demo(void);
    void test_list_demo(void);
    void test_meter_demo(void);
    void create_meter_demo(void);
    void create_table_demo(void);
    void create_tabview_demo(void);
    void create_colorwheel_demo(void);
    void create_calendar_demo(void);
    void create_spinbox_demo(void);
    void create_menu_demo(void);
    void create_tileview_demo(void);
    void create_window_demo(void);
    void run_speedometer_needle_test(void);
    
    // Phase 3: Core System
    void test_timer_demo(void);
    void test_event_demo(void);
    void test_display_demo(void);
    void test_indev_demo(void);
    void test_multitouch_demo(void);
    
    // Phase 4: Graphics
    void test_canvas_demo(void);
    void test_color_demo(void);
    
    // Phase 5: Utilities
    void test_style_demo(void);
    void test_animation_demo(void);
    
    // Phase 6: Screen Management
    void test_screen_demo(void);
    void test_screen_manager_demo(void);
    
    // Phase 7: Layout Helpers
    void test_layout_demo(void);
    
    // Phase 8: Core Framework
    void test_storage_demo(void);
    // void test_framework_demo(void);  // Removed - needs redesign
    void test_simple_note_demo(void);
    
    // Cleanup functions
    void cleanup_timer_demo(void);
    void cleanup_image_display(void);
    void cleanup_storage_demo(void);
    void cleanup_framework_demo(void);
}

// Demo item structure
typedef struct {
    const char *name;
    void (*demo_func)(void);
    const char *category;
} demo_item_t;

// List of all demos
static const demo_item_t demo_list[] = {
    // Phase 1
    {"Counter App", test_cpp_example, "Phase 1: Basic"},
    
    // Phase 2: Widgets
    {"Brightness Control (Slider)", test_brightness_control, "Phase 2: Widgets"},
    {"Switch Control", test_switch_control, "Phase 2: Widgets"},
    {"Button Control", test_button_control, "Phase 2: Widgets"},
    {"Checkbox Control", test_checkbox_control, "Phase 2: Widgets"},
    {"Dropdown Selection", test_dropdown_selection, "Phase 2: Widgets"},
    {"TextArea Editor", test_textarea_editor, "Phase 2: Widgets"},
    {"Arc Control", test_arc_control, "Phase 2: Widgets"},
    {"Bar Progress", test_bar_progress, "Phase 2: Widgets"},
    {"Chart Demo", test_chart_demo, "Phase 2: Widgets"},
    {"Roller Selection", test_roller_selection, "Phase 2: Widgets"},
    {"Image Display", test_image_display, "Phase 2: Widgets"},
    {"Spinner Demo", test_spinner_demo, "Phase 2: Widgets"},
    {"List Demo", test_list_demo, "Phase 2: Widgets"},
    {"Meter Demo", test_meter_demo, "Phase 2: Widgets"},
    {"Table Demo", create_table_demo, "Phase 2: Widgets"},
    {"TabView Demo", create_tabview_demo, "Phase 2: Widgets"},
    {"ColorWheel Demo", create_colorwheel_demo, "Phase 2: Widgets"},
    {"Calendar Demo", create_calendar_demo, "Phase 2: Widgets"},
    {"Spinbox Demo", create_spinbox_demo, "Phase 2: Widgets"},
    {"Menu Demo", create_menu_demo, "Phase 2: Widgets"},
    {"TileView Demo", create_tileview_demo, "Phase 2: Widgets"},
    {"Window Demo", create_window_demo, "Phase 2: Widgets"},
    {"Speedometer Needle", run_speedometer_needle_test, "Phase 2: Widgets"},
    
    // Phase 3: Core System
    {"Timer Demo", test_timer_demo, "Phase 3: Core System"},
    {"Event Demo", test_event_demo, "Phase 3: Core System"},
    {"Display Demo", test_display_demo, "Phase 3: Core System"},
    {"Input Device Demo", test_indev_demo, "Phase 3: Core System"},
    {"Multi-touch Demo", test_multitouch_demo, "Phase 3: Core System"},
    
    // Phase 4: Graphics
    {"Canvas Demo", test_canvas_demo, "Phase 4: Graphics"},
    {"Color Demo", test_color_demo, "Phase 4: Graphics"},
    
    // Phase 5: Utilities
    {"Style Demo", test_style_demo, "Phase 5: Utilities"},
    {"Animation Demo", test_animation_demo, "Phase 5: Utilities"},
    
    // Phase 6: Screen Management
    {"Screen Demo", test_screen_demo, "Phase 6: Screens"},
    {"Screen Manager Demo", test_screen_manager_demo, "Phase 6: Screens"},
    
    // Phase 7: Layout
    {"Layout Helpers Demo", test_layout_demo, "Phase 7: Layout"},
    
    // Phase 8: Core Framework
    {"Storage Demo (SD Card)", test_storage_demo, "Phase 8: Framework"},
    // {"Document/View Framework", test_framework_demo, "Phase 8: Framework"},  // Removed - needs redesign
    {"Simple Note (Doc/View)", test_simple_note_demo, "Phase 8: Framework"},
};

static const int demo_count = sizeof(demo_list) / sizeof(demo_list[0]);
static lv_obj_t *main_screen = NULL;
static lv_obj_t *demo_screen = NULL;

// Event handler for demo list
static void demo_list_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
        
        if (index < demo_count) {
            ESP_LOGI(TAG, "Selected demo: %s", demo_list[index].name);
            
            // Create demo screen with back button
            if (demo_screen) {
                lv_obj_del(demo_screen);
            }
            demo_screen = lv_obj_create(NULL);
            
            // Load demo screen first
            lv_scr_load(demo_screen);
            
            // Run the demo function
            if (demo_list[index].demo_func) {
                demo_list[index].demo_func();
            }
            
            // Create back button AFTER demo (so it's on top)
            lv_obj_t *back_btn = lv_btn_create(demo_screen);
            lv_obj_set_size(back_btn, 100, 50);
            lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
            
            // Make sure button is always on top
            lv_obj_move_foreground(back_btn);
            
            lv_obj_t *back_label = lv_label_create(back_btn);
            lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
            lv_obj_center(back_label);
            
            lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
                if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                    return_to_main_menu();
                }
            }, LV_EVENT_CLICKED, NULL);
        }
    }
}

void create_main_menu(void)
{
    ESP_LOGI(TAG, "Creating main menu with %d demos", demo_count);
    
    // Create main screen
    if (main_screen) {
        lv_obj_del(main_screen);
    }
    main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x1a1a2e), 0);
    
    // Create title
    lv_obj_t *title = lv_label_create(main_screen);
    lv_label_set_text(title, "LVGL C++ Demo Gallery");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Create subtitle
    lv_obj_t *subtitle = lv_label_create(main_screen);
    lv_label_set_text_fmt(subtitle, "Select a demo to run (%d available)", demo_count);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 45);
    
    // Create list container
    lv_obj_t *list_container = lv_obj_create(main_screen);
    lv_obj_set_size(list_container, lv_pct(95), lv_pct(80));
    lv_obj_align(list_container, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(list_container, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_color(list_container, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_border_width(list_container, 2, 0);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list_container, 5, 0);
    lv_obj_set_style_pad_all(list_container, 10, 0);
    
    // Add scrolling
    lv_obj_set_scroll_dir(list_container, LV_DIR_VER);
    
    // Add demos to list
    const char *last_category = "";
    for (int i = 0; i < demo_count; i++) {
        // Add category header if category changed
        if (strcmp(demo_list[i].category, last_category) != 0) {
            lv_obj_t *category_label = lv_label_create(list_container);
            lv_label_set_text(category_label, demo_list[i].category);
            lv_obj_set_style_text_color(category_label, lv_color_hex(0x3dbde6), 0);
            lv_obj_set_style_text_font(category_label, &lv_font_montserrat_16, 0);
            lv_obj_set_style_pad_top(category_label, i == 0 ? 0 : 15, 0);
            last_category = demo_list[i].category;
        }
        
        // Create demo button
        lv_obj_t *btn = lv_btn_create(list_container);
        lv_obj_set_width(btn, lv_pct(100));
        lv_obj_set_height(btn, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0f4c75), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3dbde6), LV_STATE_PRESSED);
        
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%d. %s", i + 1, demo_list[i].name);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(label);
        
        lv_obj_add_event_cb(btn, demo_list_event_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }
    
    // Load main screen
    lv_scr_load(main_screen);
    
    ESP_LOGI(TAG, "Main menu created successfully");
}

void return_to_main_menu(void)
{
    ESP_LOGI(TAG, "Returning to main menu");
    
    // Cleanup demos BEFORE cleaning screen (important!)
    cleanup_timer_demo();
    cleanup_image_display();
    cleanup_storage_demo();  // This will unmount SD card
    
    // Add delay to ensure cleanup completes
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Clean current screen first (removes all children including timers)
    lv_obj_t *current_screen = lv_scr_act();
    if (current_screen) {
        lv_obj_clean(current_screen);
    }
    
    // Delete demo screen if exists
    if (demo_screen) {
        lv_obj_del(demo_screen);
        demo_screen = NULL;
    }
    
    // Load main screen
    if (main_screen) {
        lv_scr_load(main_screen);
    } else {
        // Recreate main menu if it doesn't exist
        create_main_menu();
    }
}
