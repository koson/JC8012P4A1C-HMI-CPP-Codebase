#include "lvgl.h"
#include "Layout.hpp"
#include "esp_log.h"
#include <cstdio>

static const char* TAG = "LayoutDemo";

extern "C" void test_layout_demo()
{
    ESP_LOGI(TAG, "=== Layout Helper Demo Started ===");
    
    // Create main screen
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Layout Helpers Demo");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    Layout::align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // === Demo 1: Flex Row ===
    lv_obj_t* flexRowPanel = lv_obj_create(scr);
    Layout::setSize(flexRowPanel, LV_PCT(90), 80);
    Layout::panel(flexRowPanel, lv_color_hex(0x2c3e50), 10);
    Layout::flexRow(flexRowPanel, Layout::FlexAlign::SpaceBetween, Layout::FlexAlign::Center);
    Layout::setGap(flexRowPanel, 10);
    Layout::align(flexRowPanel, LV_ALIGN_TOP_MID, 0, 50);
    
    lv_obj_t* flexLabel = lv_label_create(flexRowPanel);
    lv_label_set_text(flexLabel, "Flex Row");
    lv_obj_set_style_text_color(flexLabel, lv_color_white(), 0);
    
    for (int i = 0; i < 3; i++) {
        lv_obj_t* box = lv_obj_create(flexRowPanel);
        Layout::setSize(box, 60, 60);
        lv_obj_set_style_bg_color(box, lv_color_hex(0x3498db), 0);
        lv_obj_set_style_radius(box, 8, 0);
        
        lv_obj_t* num = lv_label_create(box);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", i + 1);
        lv_label_set_text(num, buf);
        lv_obj_set_style_text_color(num, lv_color_white(), 0);
        Layout::center(num);
    }
    
    // === Demo 2: Flex Column ===
    lv_obj_t* flexColPanel = lv_obj_create(scr);
    Layout::setSize(flexColPanel, 150, 250);
    Layout::panel(flexColPanel, lv_color_hex(0x34495e), 10);
    Layout::flexColumn(flexColPanel, Layout::FlexAlign::SpaceAround, Layout::FlexAlign::Center);
    Layout::align(flexColPanel, LV_ALIGN_LEFT_MID, 20, 20);
    
    lv_obj_t* colLabel = lv_label_create(flexColPanel);
    lv_label_set_text(colLabel, "Flex\nColumn");
    lv_obj_set_style_text_color(colLabel, lv_color_white(), 0);
    lv_obj_set_style_text_align(colLabel, LV_TEXT_ALIGN_CENTER, 0);
    
    for (int i = 0; i < 3; i++) {
        lv_obj_t* btn = lv_button_create(flexColPanel);
        Layout::setWidth(btn, LV_PCT(90));
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xe74c3c), 0);
        
        lv_obj_t* btnLabel = lv_label_create(btn);
        lv_label_set_text(btnLabel, "Button");
        lv_obj_set_style_text_color(btnLabel, lv_color_white(), 0);
        Layout::center(btnLabel);
    }
    
    // === Demo 3: Grid Layout ===
    lv_obj_t* gridPanel = lv_obj_create(scr);
    Layout::setSize(gridPanel, 250, 250);
    Layout::panel(gridPanel, lv_color_hex(0x8e44ad), 10);
    Layout::gridEqual(gridPanel, 3, 3);
    Layout::setGap(gridPanel, 5);
    Layout::align(gridPanel, LV_ALIGN_RIGHT_MID, -20, 20);
    
    // Grid cells
    lv_color_t gridColors[] = {
        lv_color_hex(0xe74c3c), lv_color_hex(0x3498db), lv_color_hex(0x2ecc71),
        lv_color_hex(0xf39c12), lv_color_hex(0x9b59b6), lv_color_hex(0x1abc9c),
        lv_color_hex(0xd35400), lv_color_hex(0x2980b9), lv_color_hex(0x27ae60)
    };
    
    for (int i = 0; i < 9; i++) {
        lv_obj_t* cell = lv_obj_create(gridPanel);
        Layout::gridCell(cell, i % 3, i / 3);
        lv_obj_set_style_bg_color(cell, gridColors[i], 0);
        lv_obj_set_style_radius(cell, 5, 0);
        lv_obj_set_style_border_width(cell, 0, 0);
        
        lv_obj_t* cellNum = lv_label_create(cell);
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", i + 1);
        lv_label_set_text(cellNum, buf);
        lv_obj_set_style_text_color(cellNum, lv_color_white(), 0);
        lv_obj_set_style_text_font(cellNum, &lv_font_montserrat_20, 0);
        Layout::center(cellNum);
    }
    
    // === Demo 4: Card Pattern ===
    lv_obj_t* card = lv_obj_create(scr);
    Layout::setSize(card, LV_PCT(40), 120);
    Layout::card(card, 12, 15);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xecf0f1), 0);
    Layout::align(card, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    
    lv_obj_t* cardTitle = lv_label_create(card);
    lv_label_set_text(cardTitle, "Card Pattern");
    lv_obj_set_style_text_color(cardTitle, lv_color_hex(0x2c3e50), 0);
    lv_obj_set_style_text_font(cardTitle, &lv_font_montserrat_16, 0);
    Layout::align(cardTitle, LV_ALIGN_TOP_LEFT, 0, 0);
    
    lv_obj_t* cardDesc = lv_label_create(card);
    lv_label_set_text(cardDesc, "Rounded corners\nShadow effect\nPadding");
    lv_obj_set_style_text_color(cardDesc, lv_color_hex(0x7f8c8d), 0);
    lv_obj_set_style_text_font(cardDesc, &lv_font_montserrat_12, 0);
    Layout::align(cardDesc, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    
    // === Demo 5: Responsive ===
    lv_obj_t* respPanel = lv_obj_create(scr);
    Layout::responsive(respPanel);
    Layout::panel(respPanel, lv_color_hex(0x16a085), 10);
    Layout::flexRow(respPanel, Layout::FlexAlign::Center, Layout::FlexAlign::Center);
    Layout::setGap(respPanel, 10);
    Layout::align(respPanel, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    
    lv_obj_t* respLabel = lv_label_create(respPanel);
    lv_label_set_text(respLabel, "Responsive (100% width)");
    lv_obj_set_style_text_color(respLabel, lv_color_white(), 0);
    
    for (int i = 0; i < 3; i++) {
        lv_obj_t* box = lv_obj_create(respPanel);
        Layout::setSize(box, 40, 40);
        lv_obj_set_style_bg_color(box, lv_color_hex(0x2ecc71), 0);
        lv_obj_set_style_radius(box, 20, 0);
    }
    
    // === Demo 6: Flex Grow ===
    lv_obj_t* growPanel = lv_obj_create(scr);
    Layout::setSize(growPanel, LV_PCT(90), 60);
    Layout::panel(growPanel, lv_color_hex(0xc0392b), 5);
    Layout::flexRow(growPanel);
    Layout::setGap(growPanel, 5);
    Layout::align(growPanel, LV_ALIGN_TOP_MID, 0, 145);
    
    lv_obj_t* growLabel = lv_label_create(growPanel);
    lv_label_set_text(growLabel, "Flex Grow:");
    lv_obj_set_style_text_color(growLabel, lv_color_white(), 0);
    
    for (int i = 0; i < 3; i++) {
        lv_obj_t* box = lv_obj_create(growPanel);
        Layout::setHeight(box, LV_PCT(80));
        Layout::flexGrow(box, i + 1);  // Grow: 1, 2, 3
        lv_obj_set_style_bg_color(box, lv_color_hex(0xf39c12), 0);
        lv_obj_set_style_radius(box, 5, 0);
        
        lv_obj_t* growNum = lv_label_create(box);
        char buf[16];
        snprintf(buf, sizeof(buf), "x%d", i + 1);
        lv_label_set_text(growNum, buf);
        lv_obj_set_style_text_color(growNum, lv_color_white(), 0);
        Layout::center(growNum);
    }
    
    // Info
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, 
        "Flex Row | Flex Column | Grid 3x3 | Card | Responsive | Flex Grow");
    lv_obj_set_style_text_color(info, lv_color_hex(0x95a5a6), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
    Layout::align(info, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    lv_screen_load(scr);
    
    ESP_LOGI(TAG, "Layout demo created successfully");
}
