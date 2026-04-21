#include "lvgl.h"
#include "LVScreen.hpp"
#include "esp_log.h"

static const char* TAG = "ScreenDemo";

// Global screen pointers (in real app, these would be managed by ScreenManager)
static LVScreen* screen1 = nullptr;
static LVScreen* screen2 = nullptr;
static LVScreen* screen3 = nullptr;

// Forward declarations
void createScreen1();
void createScreen2();
void createScreen3();

extern "C" void test_screen_demo()
{
    ESP_LOGI(TAG, "=== LVScreen Demo Started ===");
    
    // Create all screens
    createScreen1();
    createScreen2();
    createScreen3();
    
    // Load first screen with fade-in
    screen1->load(LVScreen::Transition::FadeIn, 500);
}

void createScreen1()
{
    screen1 = new LVScreen();
    screen1->setBackgroundColor(lv_color_hex(0x2c3e50))
           .onLoad([]() {
               ESP_LOGI(TAG, "✓ Screen 1 loaded");
           })
           .onUnload([]() {
               ESP_LOGI(TAG, "✗ Screen 1 unloaded");
           });
    
    lv_obj_t* scr = screen1->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Screen 1: Transitions");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Subtitle
    lv_obj_t* subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Try different transition effects");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xbdc3c7), 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_16, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 60);
    
    // Container for buttons
    lv_obj_t* btn_container = lv_obj_create(scr);
    lv_obj_set_size(btn_container, LV_PCT(90), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_style_pad_all(btn_container, 10, 0);
    lv_obj_align(btn_container, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Fade In button
    lv_obj_t* btnFade = lv_button_create(btn_container);
    lv_obj_set_size(btnFade, 300, 50);
    lv_obj_set_style_bg_color(btnFade, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(btnFade, 8, 0);
    
    lv_obj_t* lblFade = lv_label_create(btnFade);
    lv_label_set_text(lblFade, "-> Fade to Screen 2");
    lv_obj_set_style_text_color(lblFade, lv_color_white(), 0);
    lv_obj_center(lblFade);
    
    lv_obj_add_event_cb(btnFade, [](lv_event_t* e) {
        screen2->load(LVScreen::Transition::FadeIn, 400);
    }, LV_EVENT_CLICKED, nullptr);
    
    // Slide Left button
    lv_obj_t* btnSlide = lv_button_create(btn_container);
    lv_obj_set_size(btnSlide, 300, 50);
    lv_obj_set_style_bg_color(btnSlide, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(btnSlide, 8, 0);
    
    lv_obj_t* lblSlide = lv_label_create(btnSlide);
    lv_label_set_text(lblSlide, "<- Slide to Screen 2");
    lv_obj_set_style_text_color(lblSlide, lv_color_white(), 0);
    lv_obj_center(lblSlide);
    
    lv_obj_add_event_cb(btnSlide, [](lv_event_t* e) {
        screen2->load(LVScreen::Transition::MoveLeft, 400);
    }, LV_EVENT_CLICKED, nullptr);
    
    // Over Top button
    lv_obj_t* btnOver = lv_button_create(btn_container);
    lv_obj_set_size(btnOver, 300, 50);
    lv_obj_set_style_bg_color(btnOver, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(btnOver, 8, 0);
    
    lv_obj_t* lblOver = lv_label_create(btnOver);
    lv_label_set_text(lblOver, "^ Over from Screen 2");
    lv_obj_set_style_text_color(lblOver, lv_color_white(), 0);
    lv_obj_center(lblOver);
    
    lv_obj_add_event_cb(btnOver, [](lv_event_t* e) {
        screen2->load(LVScreen::Transition::OverTop, 400);
    }, LV_EVENT_CLICKED, nullptr);
    
    // Info
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, "Each button uses a different transition effect");
    lv_obj_set_style_text_color(info, lv_color_hex(0x95a5a6), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_12, 0);
    lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void createScreen2()
{
    screen2 = new LVScreen();
    screen2->setBackgroundColor(lv_color_hex(0x8e44ad))
           .onLoad([]() {
               ESP_LOGI(TAG, "✓ Screen 2 loaded");
           })
           .onUnload([]() {
               ESP_LOGI(TAG, "✗ Screen 2 unloaded");
           });
    
    lv_obj_t* scr = screen2->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Screen 2: Configuration");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Description
    lv_obj_t* desc = lv_label_create(scr);
    lv_label_set_text(desc, "This screen has a purple background\nand demonstrates callbacks");
    lv_obj_set_style_text_color(desc, lv_color_hex(0xecf0f1), 0);
    lv_obj_set_style_text_font(desc, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(desc, LV_ALIGN_TOP_MID, 0, 70);
    
    // Container
    lv_obj_t* container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_PCT(85), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x9b59b6), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_set_style_border_color(container, lv_color_hex(0xecf0f1), 0);
    lv_obj_set_style_radius(container, 10, 0);
    lv_obj_set_style_pad_all(container, 20, 0);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Button to screen 1
    lv_obj_t* btnToScreen1 = lv_button_create(container);
    lv_obj_set_size(btnToScreen1, 280, 50);
    lv_obj_set_style_bg_color(btnToScreen1, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_radius(btnToScreen1, 8, 0);
    
    lv_obj_t* lbl1 = lv_label_create(btnToScreen1);
    lv_label_set_text(lbl1, "<- Back to Screen 1");
    lv_obj_set_style_text_color(lbl1, lv_color_white(), 0);
    lv_obj_center(lbl1);
    
    lv_obj_add_event_cb(btnToScreen1, [](lv_event_t* e) {
        screen1->load(LVScreen::Transition::MoveRight, 400);
    }, LV_EVENT_CLICKED, nullptr);
    
    // Button to screen 3
    lv_obj_t* btnToScreen3 = lv_button_create(container);
    lv_obj_set_size(btnToScreen3, 280, 50);
    lv_obj_set_style_bg_color(btnToScreen3, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_radius(btnToScreen3, 8, 0);
    
    lv_obj_t* lbl3 = lv_label_create(btnToScreen3);
    lv_label_set_text(lbl3, "-> Next to Screen 3");
    lv_obj_set_style_text_color(lbl3, lv_color_white(), 0);
    lv_obj_center(lbl3);
    
    lv_obj_add_event_cb(btnToScreen3, [](lv_event_t* e) {
        screen3->load(LVScreen::Transition::MoveLeft, 400);
    }, LV_EVENT_CLICKED, nullptr);
}

void createScreen3()
{
    screen3 = new LVScreen();
    screen3->setBackgroundColor(lv_color_hex(0x16a085))
           .onLoad([]() {
               ESP_LOGI(TAG, "✓ Screen 3 loaded - Final screen!");
           })
           .onUnload([]() {
               ESP_LOGI(TAG, "✗ Screen 3 unloaded");
           });
    
    lv_obj_t* scr = screen3->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Screen 3: Complete!");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);
    
    // Success message
    lv_obj_t* msg = lv_label_create(scr);
    lv_label_set_text(msg, 
        "LVScreen Demo Complete!\n\n"
        "Features demonstrated:\n"
        "- Multiple screens\n"
        "- Transition effects\n"
        "- Lifecycle callbacks\n"
        "- Background colors\n"
        "- Method chaining");
    lv_obj_set_style_text_color(msg, lv_color_white(), 0);
    lv_obj_set_style_text_font(msg, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 0);
    
    // Back button
    lv_obj_t* btnBack = lv_button_create(scr);
    lv_obj_set_size(btnBack, 280, 50);
    lv_obj_set_style_bg_color(btnBack, lv_color_hex(0x2ecc71), 0);
    lv_obj_set_style_radius(btnBack, 8, 0);
    lv_obj_align(btnBack, LV_ALIGN_BOTTOM_MID, 0, -40);
    
    lv_obj_t* lblBack = lv_label_create(btnBack);
    lv_label_set_text(lblBack, "Back to Start");
    lv_obj_set_style_text_color(lblBack, lv_color_white(), 0);
    lv_obj_center(lblBack);
    
    lv_obj_add_event_cb(btnBack, [](lv_event_t* e) {
        screen1->load(LVScreen::Transition::FadeIn, 500);
    }, LV_EVENT_CLICKED, nullptr);
}
