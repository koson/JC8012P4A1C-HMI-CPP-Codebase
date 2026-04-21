/**
 * @file menu_demo.cpp
 * @brief Demo for LVMenu - Hierarchical menu widget testing
 * 
 * Tests:
 * - Menu with nested pages
 * - Back button navigation
 * - Menu sections
 * - Container items (navigate to sub-page)
 * - Switch items
 */

#include "lvgl.h"

static lv_obj_t* create_text(lv_obj_t* parent, const char* icon, const char* txt)
{
    lv_obj_t* obj = lv_menu_cont_create(parent);
    
    lv_obj_t* img = NULL;
    lv_obj_t* label = NULL;
    
    if(icon) {
        img = lv_image_create(obj);
        lv_image_set_src(img, icon);
    }
    
    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }
    
    return obj;
}

static lv_obj_t* create_slider(lv_obj_t* parent, const char* icon, const char* txt, int32_t min, int32_t max, int32_t val)
{
    lv_obj_t* obj = create_text(parent, icon, txt);
    
    lv_obj_t* slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);
    
    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }
    
    return obj;
}

static lv_obj_t* create_switch(lv_obj_t* parent, const char* icon, const char* txt, bool checked)
{
    lv_obj_t* obj = create_text(parent, icon, txt);
    
    lv_obj_t* sw = lv_switch_create(obj);
    if(checked) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    
    return obj;
}

extern "C" void create_menu_demo() {
    lv_obj_t* menu = lv_menu_create(lv_screen_active());
    lv_obj_set_size(menu, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL));
    lv_obj_center(menu);
    
    lv_obj_t* cont;
    lv_obj_t* section;
    
    // Create main page
    lv_obj_t* main_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(main_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), LV_PART_MAIN), 0);
    section = lv_menu_section_create(main_page);
    
    // Settings section
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Settings");
    lv_obj_t* settings_page = lv_menu_page_create(menu, "Settings");
    lv_menu_set_load_page_event(menu, cont, settings_page);
    
    // WiFi section  
    cont = create_text(section, LV_SYMBOL_WIFI, "WiFi");
    lv_obj_t* wifi_page = lv_menu_page_create(menu, "WiFi");
    lv_menu_set_load_page_event(menu, cont, wifi_page);
    
    // Bluetooth section
    cont = create_text(section, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_t* bt_page = lv_menu_page_create(menu, "Bluetooth");
    lv_menu_set_load_page_event(menu, cont, bt_page);
    
    // About section
    cont = create_text(section, LV_SYMBOL_LIST, "About");
    lv_obj_t* about_page = lv_menu_page_create(menu, "About");
    lv_menu_set_load_page_event(menu, cont, about_page);
    
    // ===== Settings Page =====
    section = lv_menu_section_create(settings_page);
    create_switch(section, LV_SYMBOL_EYE_OPEN, "Auto Brightness", true);
    create_slider(section, LV_SYMBOL_IMAGE, "Brightness", 0, 100, 75);
    create_switch(section, LV_SYMBOL_BELL, "Notifications", false);
    
    // ===== WiFi Page =====
    section = lv_menu_section_create(wifi_page);
    create_switch(section, LV_SYMBOL_WIFI, "WiFi Enable", true);
    create_slider(section, LV_SYMBOL_CHARGE, "Signal Strength", 0, 100, 85);
    
    // ===== Bluetooth Page =====
    section = lv_menu_section_create(bt_page);
    create_switch(section, LV_SYMBOL_BLUETOOTH, "BT Enable", false);
    create_slider(section, LV_SYMBOL_VOLUME_MAX, "Volume", 0, 100, 50);
    
    // ===== About Page =====
    section = lv_menu_section_create(about_page);
    lv_obj_t* info_label = lv_label_create(section);
    lv_label_set_text(info_label, 
        "System Information\n\n"
        "Version: 1.0.0\n"
        "Build: 2026.01.15\n"
        "Device: ESP32-P4\n"
        "LVGL: v9.2.2");
    
    // Set main page
    lv_menu_set_page(menu, main_page);
}
