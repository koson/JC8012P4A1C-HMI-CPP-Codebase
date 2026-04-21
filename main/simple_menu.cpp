#include "esp_log.h"
#include "lvgl.h"

static const char* TAG = "SimpleMenu";

// Forward declaration
void create_simple_menu(void);

// Demo list structure
typedef struct {
    const char* name;
    const char* description;
} simple_demo_t;

static const simple_demo_t demo_list[] = {
    {"Widget Gallery", "Browse LVGL widgets"},
    {"Touch Test", "Test touch functionality"},
    {"Display Test", "Test display features"},
    {"Simple Animation", "Basic animation demo"},
    {"Color Test", "Display color patterns"},
};

static const int demo_count = sizeof(demo_list) / sizeof(demo_list[0]);

// ========== Demo Implementation Functions ==========

// Demo 0: Widget Gallery
static void create_widget_gallery(lv_obj_t* parent)
{
    // Slider
    lv_obj_t* slider_label = lv_label_create(parent);
    lv_label_set_text(slider_label, "Slider:");
    lv_obj_align(slider_label, LV_ALIGN_TOP_LEFT, 20, 100);
    
    lv_obj_t* slider = lv_slider_create(parent);
    lv_obj_set_width(slider, 300);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 120, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    
    lv_obj_t* slider_value = lv_label_create(parent);
    lv_label_set_text(slider_value, "50%");
    lv_obj_align(slider_value, LV_ALIGN_TOP_LEFT, 450, 100);
    
    lv_obj_add_event_cb(slider, [](lv_event_t* e) {
        lv_obj_t* slider = static_cast<lv_obj_t*>(lv_event_get_target(e));
        lv_obj_t* label = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        int32_t value = lv_slider_get_value(slider);
        lv_label_set_text_fmt(label, "%d%%", (int)value);
    }, LV_EVENT_VALUE_CHANGED, slider_value);
    
    // Switch
    lv_obj_t* switch_label = lv_label_create(parent);
    lv_label_set_text(switch_label, "Switch:");
    lv_obj_align(switch_label, LV_ALIGN_TOP_LEFT, 20, 160);
    
    lv_obj_t* sw = lv_switch_create(parent);
    lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 120, 155);
    
    lv_obj_t* switch_status = lv_label_create(parent);
    lv_label_set_text(switch_status, "OFF");
    lv_obj_align(switch_status, LV_ALIGN_TOP_LEFT, 240, 160);
    
    lv_obj_add_event_cb(sw, [](lv_event_t* e) {
        lv_obj_t* sw = static_cast<lv_obj_t*>(lv_event_get_target(e));
        lv_obj_t* label = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        lv_label_set_text(label, lv_obj_has_state(sw, LV_STATE_CHECKED) ? "ON" : "OFF");
    }, LV_EVENT_VALUE_CHANGED, switch_status);
    
    // Buttons
    lv_obj_t* btn1 = lv_btn_create(parent);
    lv_obj_set_size(btn1, 150, 60);
    lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 20, 230);
    lv_obj_t* btn1_label = lv_label_create(btn1);
    lv_label_set_text(btn1_label, LV_SYMBOL_OK " Confirm");
    lv_obj_center(btn1_label);
    
    lv_obj_t* btn2 = lv_btn_create(parent);
    lv_obj_set_size(btn2, 150, 60);
    lv_obj_align(btn2, LV_ALIGN_TOP_LEFT, 190, 230);
    lv_obj_t* btn2_label = lv_label_create(btn2);
    lv_label_set_text(btn2_label, LV_SYMBOL_CLOSE " Cancel");
    lv_obj_center(btn2_label);
    
    // Checkbox
    lv_obj_t* cb1 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb1, "Option 1");
    lv_obj_align(cb1, LV_ALIGN_TOP_LEFT, 20, 320);
    
    lv_obj_t* cb2 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb2, "Option 2");
    lv_obj_align(cb2, LV_ALIGN_TOP_LEFT, 20, 370);
    lv_obj_add_state(cb2, LV_STATE_CHECKED);
}

// Demo 1: Touch Test
static lv_obj_t* touch_indicator = nullptr;
static lv_obj_t* touch_coord_label = nullptr;

static void create_touch_test(lv_obj_t* parent)
{
    lv_obj_t* info = lv_label_create(parent);
    lv_label_set_text(info, "Touch anywhere on the screen");
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);
    
    touch_coord_label = lv_label_create(parent);
    lv_label_set_text(touch_coord_label, "No touch detected");
    lv_obj_set_style_text_font(touch_coord_label, &lv_font_montserrat_24, 0);
    lv_obj_align(touch_coord_label, LV_ALIGN_CENTER, 0, 0);
    
    // Create touch indicator
    touch_indicator = lv_obj_create(parent);
    lv_obj_set_size(touch_indicator, 60, 60);
    lv_obj_set_style_radius(touch_indicator, 30, 0);
    lv_obj_set_style_bg_color(touch_indicator, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_border_width(touch_indicator, 4, 0);
    lv_obj_set_style_border_color(touch_indicator, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);
    
    // Add touch event to parent
    lv_obj_add_event_cb(parent, [](lv_event_t* e) {
        lv_event_code_t code = lv_event_get_code(e);
        lv_indev_t* indev = lv_indev_active();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        
        if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
            lv_obj_clear_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(touch_indicator, point.x - 30, point.y - 30);
            lv_label_set_text_fmt(touch_coord_label, "X: %d, Y: %d", point.x, point.y);
        } else if (code == LV_EVENT_RELEASED) {
            lv_obj_add_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_ALL, nullptr);
}

// Demo 2: Display Test
static void create_display_test(lv_obj_t* parent)
{
    lv_obj_t* info = lv_label_create(parent);
    lv_label_set_text(info, "Display Resolution & Color Bars");
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);
    
    // Display info
    lv_obj_t* res_label = lv_label_create(parent);
    lv_label_set_text_fmt(res_label, "Resolution: %d x %d", LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_text_font(res_label, &lv_font_montserrat_20, 0);
    lv_obj_align(res_label, LV_ALIGN_TOP_MID, 0, 120);
    
    // Color bars
    const lv_color_t colors[] = {
        lv_color_hex(0xFF0000), // Red
        lv_color_hex(0x00FF00), // Green
        lv_color_hex(0x0000FF), // Blue
        lv_color_hex(0xFFFF00), // Yellow
        lv_color_hex(0xFF00FF), // Magenta
        lv_color_hex(0x00FFFF), // Cyan
        lv_color_hex(0xFFFFFF), // White
    };
    
    int bar_height = 60;
    int start_y = 200;
    
    for (int i = 0; i < 7; i++) {
        lv_obj_t* bar = lv_obj_create(parent);
        lv_obj_set_size(bar, 600, bar_height);
        lv_obj_set_pos(bar, (LV_HOR_RES - 600) / 2, start_y + (i * (bar_height + 5)));
        lv_obj_set_style_bg_color(bar, colors[i], 0);
        lv_obj_set_style_border_width(bar, 0, 0);
        lv_obj_set_style_pad_all(bar, 0, 0);
    }
}

// Demo 3: Simple Animation
static lv_obj_t* anim_obj = nullptr;

static void create_simple_animation(lv_obj_t* parent)
{
    lv_obj_t* info = lv_label_create(parent);
    lv_label_set_text(info, "Bouncing Ball Animation");
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);
    
    // Create animated object
    anim_obj = lv_obj_create(parent);
    lv_obj_set_size(anim_obj, 80, 80);
    lv_obj_set_style_radius(anim_obj, 40, 0);
    lv_obj_set_style_bg_color(anim_obj, lv_color_hex(0xFF5722), 0);
    lv_obj_set_pos(anim_obj, 100, 200);
    
    // Create animation
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, anim_obj);
    lv_anim_set_values(&anim, 100, LV_HOR_RES - 180);
    lv_anim_set_duration(&anim, 2000);
    lv_anim_set_playback_duration(&anim, 2000);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
        lv_obj_set_x((lv_obj_t*)var, value);
    });
    lv_anim_start(&anim);
    
    // Y animation
    lv_anim_t anim_y;
    lv_anim_init(&anim_y);
    lv_anim_set_var(&anim_y, anim_obj);
    lv_anim_set_values(&anim_y, 200, LV_VER_RES - 280);
    lv_anim_set_duration(&anim_y, 1500);
    lv_anim_set_playback_duration(&anim_y, 1500);
    lv_anim_set_repeat_count(&anim_y, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim_y, lv_anim_path_bounce);
    lv_anim_set_exec_cb(&anim_y, [](void* var, int32_t value) {
        lv_obj_set_y((lv_obj_t*)var, value);
    });
    lv_anim_start(&anim_y);
}

// Demo 4: Color Test
static void create_color_test(lv_obj_t* parent)
{
    lv_obj_t* info = lv_label_create(parent);
    lv_label_set_text(info, "RGB Color Gradient");
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);
    
    // Create gradient boxes
    int box_size = 100;
    int spacing = 20;
    int start_x = 100;
    int start_y = 150;
    
    // Red gradient
    for (int i = 0; i < 5; i++) {
        lv_obj_t* box = lv_obj_create(parent);
        lv_obj_set_size(box, box_size, box_size);
        lv_obj_set_pos(box, start_x + (i * (box_size + spacing)), start_y);
        lv_obj_set_style_bg_color(box, lv_color_hex(0xFF0000 | (i * 0x003333)), 0);
        lv_obj_set_style_border_width(box, 2, 0);
    }
    
    // Green gradient
    start_y = 280;
    for (int i = 0; i < 5; i++) {
        lv_obj_t* box = lv_obj_create(parent);
        lv_obj_set_size(box, box_size, box_size);
        lv_obj_set_pos(box, start_x + (i * (box_size + spacing)), start_y);
        lv_obj_set_style_bg_color(box, lv_color_hex(0x00FF00 | (i * 0x330033)), 0);
        lv_obj_set_style_border_width(box, 2, 0);
    }
    
    // Blue gradient
    start_y = 410;
    for (int i = 0; i < 5; i++) {
        lv_obj_t* box = lv_obj_create(parent);
        lv_obj_set_size(box, box_size, box_size);
        lv_obj_set_pos(box, start_x + (i * (box_size + spacing)), start_y);
        lv_obj_set_style_bg_color(box, lv_color_hex(0x0000FF | (i * 0x333300)), 0);
        lv_obj_set_style_border_width(box, 2, 0);
    }
}

// Event handler for demo selection
static void demo_list_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        uint32_t index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
        
        if (index < demo_count) {
            ESP_LOGI(TAG, "Selected: %s", demo_list[index].name);
            
            // Create demo screen
            lv_obj_t* demo_screen = lv_obj_create(NULL);
            lv_obj_set_style_bg_color(demo_screen, lv_color_hex(0x1a1a2e), 0);
            lv_scr_load(demo_screen);
            
            // Title
            lv_obj_t* title = lv_label_create(demo_screen);
            lv_label_set_text(title, demo_list[index].name);
            lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
            lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
            lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
            
            // Create demo content based on index
            switch (index) {
                case 0: create_widget_gallery(demo_screen); break;
                case 1: create_touch_test(demo_screen); break;
                case 2: create_display_test(demo_screen); break;
                case 3: create_simple_animation(demo_screen); break;
                case 4: create_color_test(demo_screen); break;
            }
            
            // Add back button (always on top)
            lv_obj_t* back_btn = lv_btn_create(demo_screen);
            lv_obj_set_size(back_btn, 120, 60);
            lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
            lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x0f4c75), 0);
            
            lv_obj_t* back_label = lv_label_create(back_btn);
            lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
            lv_obj_center(back_label);
            
            lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
                if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                    // Stop any running animations before going back
                    lv_anim_delete_all();
                    create_simple_menu();
                }
            }, LV_EVENT_CLICKED, NULL);
        }
    }
}
// Forward declaration for external use
// void create_simple_menu(void);

void create_simple_menu(void)
{
    ESP_LOGI(TAG, "Creating simple demo menu (%d items)", demo_count);
    
    // Create main screen
    lv_obj_t* main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x1a1a2e), 0);
    lv_scr_load(main_screen);
    
    // Create title
    lv_obj_t* title = lv_label_create(main_screen);
    lv_label_set_text(title, "ESP32-P4 Demo Gallery");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create subtitle
    lv_obj_t* subtitle = lv_label_create(main_screen);
    lv_label_set_text(subtitle, "Select a demo to explore");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 60);
    
    // Create list container
    lv_obj_t* list_container = lv_obj_create(main_screen);
    lv_obj_set_size(list_container, lv_pct(90), lv_pct(70));
    lv_obj_align(list_container, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_bg_color(list_container, lv_color_hex(0x16213e), 0);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list_container, 10, 0);
    lv_obj_set_style_pad_all(list_container, 20, 0);
    lv_obj_set_scroll_dir(list_container, LV_DIR_VER);
    
    // Add demo buttons
    for (int i = 0; i < demo_count; i++) {
        lv_obj_t* btn = lv_btn_create(list_container);
        lv_obj_set_width(btn, lv_pct(100));
        lv_obj_set_height(btn, 80);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0f4c75), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3dbde6), LV_STATE_PRESSED);
        
        // Demo name label
        lv_obj_t* name_label = lv_label_create(btn);
        lv_label_set_text_fmt(name_label, "%d. %s", i + 1, demo_list[i].name);
        lv_obj_set_style_text_font(name_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(name_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, 10, 10);
        
        // Description label
        lv_obj_t* desc_label = lv_label_create(btn);
        lv_label_set_text(desc_label, demo_list[i].description);
        lv_obj_set_style_text_color(desc_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(desc_label, LV_ALIGN_BOTTOM_LEFT, 10, -10);
        
        lv_obj_add_event_cb(btn, demo_list_event_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }
    
    ESP_LOGI(TAG, "✓ Simple menu created successfully");
}
