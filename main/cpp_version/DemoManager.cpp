#include "DemoManager.h"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "DemoManager";

// ========== Constructor & Destructor ==========

DemoManager::DemoManager()
    : menu_screen(nullptr), current_demo_screen(nullptr)
{
    ESP_LOGI(TAG, "DemoManager created");
}

DemoManager::~DemoManager()
{
    clearDemos();
    ESP_LOGI(TAG, "DemoManager destroyed");
}

// ========== Public Methods ==========

void DemoManager::registerDemo(const char* name, const char* description,
                               std::function<void(lv_obj_t*)> createFunction)
{
    demos.emplace_back(name, description, createFunction);
    ESP_LOGI(TAG, "Registered demo: %s (total: %d)", name, demos.size());
}

void DemoManager::showMenu()
{
    // Clean up previous demo screen
    if (current_demo_screen) {
        lv_anim_delete_all();
        lv_obj_delete(current_demo_screen);
        current_demo_screen = nullptr;
    }
    
    createMenuUI();
    ESP_LOGI(TAG, "Menu shown with %d demos", demos.size());
}

void DemoManager::clearDemos()
{
    demos.clear();
    if (menu_screen) {
        lv_obj_delete(menu_screen);
        menu_screen = nullptr;
    }
    if (current_demo_screen) {
        lv_obj_delete(current_demo_screen);
        current_demo_screen = nullptr;
    }
}

// ========== Private Methods ==========

void DemoManager::createMenuUI()
{
    // Create menu screen
    menu_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(menu_screen, lv_color_hex(0x0f0f23), 0);
    lv_scr_load(menu_screen);
    
    // Title
    lv_obj_t* title = lv_label_create(menu_screen);
    lv_label_set_text(title, "LVGL Demo Gallery");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Subtitle
    lv_obj_t* subtitle = lv_label_create(menu_screen);
    lv_label_set_text_fmt(subtitle, "%d Examples Available", demos.size());
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 60);
    
    // Create scrollable container for demo list
    lv_obj_t* list_container = lv_obj_create(menu_screen);
    lv_obj_set_size(list_container, LV_HOR_RES - 100, LV_VER_RES - 180);
    lv_obj_set_pos(list_container, 50, 100);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(list_container, 15, 0);
    lv_obj_set_style_bg_color(list_container, lv_color_hex(0x1a1a2e), 0);
    
    // Create demo items
    for (size_t i = 0; i < demos.size(); i++) {
        lv_obj_t* item = lv_obj_create(list_container);
        lv_obj_set_size(item, LV_HOR_RES - 140, 80);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x16213e), 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0x0f4c75), 0);
        lv_obj_set_style_border_width(item, 2, 0);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
        
        // Hover effect
        lv_obj_set_style_bg_color(item, lv_color_hex(0x1e3a5f), LV_STATE_PRESSED);
        
        // Demo name
        lv_obj_t* name_label = lv_label_create(item);
        lv_label_set_text(name_label, demos[i].name.c_str());
        lv_obj_set_style_text_font(name_label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(name_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, 15, 10);
        
        // Demo description
        lv_obj_t* desc_label = lv_label_create(item);
        lv_label_set_text(desc_label, demos[i].description.c_str());
        lv_obj_set_style_text_color(desc_label, lv_color_hex(0x888888), 0);
        lv_obj_align(desc_label, LV_ALIGN_TOP_LEFT, 15, 40);
        
        // Add click event
        lv_obj_set_user_data(item, reinterpret_cast<void*>(i));
        lv_obj_add_event_cb(item, menuEventHandler, LV_EVENT_CLICKED, this);
    }
}

void DemoManager::createDemoScreen(size_t index)
{
    if (index >= demos.size()) {
        ESP_LOGE(TAG, "Invalid demo index: %d", index);
        return;
    }
    
    // Clean up animations from previous demo
    lv_anim_delete_all();
    
    // Create demo screen
    current_demo_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(current_demo_screen, lv_color_hex(0x1a1a2e), 0);
    lv_scr_load(current_demo_screen);
    
    // Title
    lv_obj_t* title = lv_label_create(current_demo_screen);
    lv_label_set_text(title, demos[index].name.c_str());
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Call demo creation function
    demos[index].createFunction(current_demo_screen);
    
    // Back button (always on top)
    lv_obj_t* back_btn = lv_btn_create(current_demo_screen);
    lv_obj_set_size(back_btn, 120, 60);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x0f4c75), 0);
    
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
    lv_obj_center(back_label);
    
    lv_obj_add_event_cb(back_btn, backButtonHandler, LV_EVENT_CLICKED, this);
    
    ESP_LOGI(TAG, "Loaded demo: %s", demos[index].name.c_str());
}

// ========== Static Event Handlers ==========

void DemoManager::menuEventHandler(lv_event_t* e)
{
    lv_obj_t* item = static_cast<lv_obj_t*>(lv_event_get_target(e));
    DemoManager* manager = static_cast<DemoManager*>(lv_event_get_user_data(e));
    size_t index = reinterpret_cast<uintptr_t>(lv_obj_get_user_data(item));
    
    manager->createDemoScreen(index);
}

void DemoManager::backButtonHandler(lv_event_t* e)
{
    DemoManager* manager = static_cast<DemoManager*>(lv_event_get_user_data(e));
    manager->showMenu();
}
