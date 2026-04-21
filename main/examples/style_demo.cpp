#include "lvgl.h"
#include "LVStyle.hpp"
#include "LVColor.hpp"
#include "LVDisplay.hpp"

// Create reusable style presets
static LVStyle* dark_panel_style = nullptr;
static LVStyle* accent_button_style = nullptr;
static LVStyle* title_text_style = nullptr;
static LVStyle* card_style = nullptr;

static void create_style_presets()
{
    // Dark panel style
    dark_panel_style = new LVStyle();
    dark_panel_style->setBackgroundColor(LVColor(0x1E1E1E))
                     .setBackgroundOpacity(LV_OPA_COVER)
                     .setBorderColor(LVColor(0x444444))
                     .setBorderWidth(2)
                     .setRadius(8)
                     .setPadding(12);
    
    // Accent button style
    accent_button_style = new LVStyle();
    accent_button_style->setBackgroundColor(LVColor(0x2196F3))
                        .setBackgroundOpacity(LV_OPA_COVER)
                        .setTextColor(LVColor::White)
                        .setRadius(6)
                        .setPadding(10, 20, 10, 20)
                        .setShadowColor(LVColor(0x000000))
                        .setShadowWidth(8)
                        .setShadowOffset(0, 4)
                        .setShadowOpacity(LV_OPA_50);
    
    // Title text style
    title_text_style = new LVStyle();
    title_text_style->setTextColor(LVColor(0xFFFFFF))
                     .setTextFont(&lv_font_montserrat_20);
    
    // Card style with gradient
    card_style = new LVStyle();
    card_style->setBackgroundColor(LVColor(0x2C2C2C))
              .setBackgroundGradientColor(LVColor(0x1A1A1A))
              .setBackgroundGradientDirection(LV_GRAD_DIR_VER)
              .setBackgroundOpacity(LV_OPA_COVER)
              .setBorderColor(LVColor(0x555555))
              .setBorderWidth(1)
              .setRadius(12)
              .setPadding(16)
              .setShadowColor(LVColor::Black)
              .setShadowWidth(10)
              .setShadowOffset(0, 2)
              .setShadowOpacity(LV_OPA_30);
}

static void button_clicked(lv_event_t* e)
{
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
    static int count = 0;
    count++;
    lv_label_set_text_fmt(label, "Clicked %d times", count);
}

extern "C" void test_style_demo()
{
    create_style_presets();
    
    LVDisplay& display = LVDisplay::getDefault();
    
    // Create main screen
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0A0A0A), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LVStyle Demo");
    title_text_style->applyTo(title);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Card container
    lv_obj_t* card = lv_obj_create(scr);
    lv_obj_set_size(card, 400, 300);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    card_style->applyTo(card);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    
    // Card title
    lv_obj_t* card_title = lv_label_create(card);
    lv_label_set_text(card_title, "Styled Card Container");
    lv_obj_set_style_text_color(card_title, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(card_title, &lv_font_montserrat_16, 0);
    lv_obj_align(card_title, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Dark panel inside card
    lv_obj_t* panel = lv_obj_create(card);
    lv_obj_set_size(panel, LV_PCT(100), 120);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 40);
    dark_panel_style->applyTo(panel);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    
    // Panel content
    lv_obj_t* panel_label = lv_label_create(panel);
    lv_label_set_text(panel_label, "Dark Panel with:\n• Border\n• Padding\n• Rounded corners");
    lv_obj_set_style_text_color(panel_label, lv_color_hex(0xAAAAAA), 0);
    lv_obj_center(panel_label);
    
    // Accent button
    lv_obj_t* btn = lv_btn_create(card);
    lv_obj_set_size(btn, 160, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    accent_button_style->applyTo(btn);
    
    lv_obj_t* btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click Me!");
    lv_obj_center(btn_label);
    
    // Click counter label
    lv_obj_t* counter = lv_label_create(scr);
    lv_label_set_text(counter, "Click the button!");
    lv_obj_set_style_text_color(counter, lv_color_hex(0x888888), 0);
    lv_obj_align(counter, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    lv_obj_add_event_cb(btn, button_clicked, LV_EVENT_CLICKED, counter);
    
    // Style comparison panel
    lv_obj_t* comparison = lv_obj_create(scr);
    lv_obj_set_size(comparison, 380, 80);
    lv_obj_align(comparison, LV_ALIGN_BOTTOM_LEFT, 20, -80);
    
    // Different radius examples
    LVStyle radius_styles[4];
    const char* labels[] = {"r=0", "r=4", "r=8", "r=16"};
    int32_t radii[] = {0, 4, 8, 16};
    
    for (int i = 0; i < 4; i++) {
        radius_styles[i].setBackgroundColor(LVColor(0x4CAF50))
                        .setBackgroundOpacity(LV_OPA_COVER)
                        .setRadius(radii[i])
                        .setPadding(8);
        
        lv_obj_t* box = lv_obj_create(comparison);
        lv_obj_set_size(box, 70, 50);
        lv_obj_set_pos(box, 10 + i * 85, 15);
        radius_styles[i].applyTo(box);
        
        lv_obj_t* lbl = lv_label_create(box);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
    }
    
    display.setActiveScreen(scr);
}
