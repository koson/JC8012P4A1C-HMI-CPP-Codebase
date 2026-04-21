/**
 * @file button_control.cpp
 * @brief Phase 2 Testing: LVButton Comprehensive Testing
 * 
 * Test Cases:
 * - Multiple button creation with different styles
 * - Button click event handling
 * - Button states (enabled/disabled)
 * - Button with embedded labels
 * - Different sizes and positions
 * - Color schemes and visual feedback
 */

#include "lvgl.h"
#include "LVWidget.hpp"
#include "LVButton.hpp"
#include "LVLabel.hpp"
#include "LVPanel.hpp"
#include "esp_log.h"
#include <stdio.h>

static const char* TAG = "BUTTON_TEST";

// UI Components
static LVButton* btnPrimary = nullptr;
static LVButton* btnSecondary = nullptr;
static LVButton* btnDanger = nullptr;
static LVButton* btnDisabled = nullptr;
static LVLabel* statusLabel = nullptr;
static int clickCounter = 0;

/**
 * Primary button click handler
 */
static void onPrimaryClick(lv_event_t* e) {
    clickCounter++;
    ESP_LOGI(TAG, "Primary Button Clicked! (Count: %d)", clickCounter);
    
    if (statusLabel) {
        char text[64];
        snprintf(text, sizeof(text), "Primary clicked: %d times", clickCounter);
        statusLabel->setText(text);
        lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(0, 150, 255), 0);
    }
}

/**
 * Secondary button click handler
 */
static void onSecondaryClick(lv_event_t* e) {
    ESP_LOGI(TAG, "Secondary Button Clicked!");
    
    if (statusLabel) {
        statusLabel->setText("Secondary button pressed");
        lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(100, 100, 100), 0);
    }
}

/**
 * Danger button click handler
 */
static void onDangerClick(lv_event_t* e) {
    ESP_LOGI(TAG, "Danger Button Clicked!");
    
    if (statusLabel) {
        statusLabel->setText("⚠️ DANGER ACTION!");
        lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(255, 0, 0), 0);
    }
    
    // Toggle disabled button state as demo
    if (btnDisabled) {
        static bool isDisabled = true;
        isDisabled = !isDisabled;
        
        if (isDisabled) {
            lv_obj_add_state(btnDisabled->obj(), LV_STATE_DISABLED);
            ESP_LOGI(TAG, "Disabled button is now DISABLED");
        } else {
            lv_obj_clear_state(btnDisabled->obj(), LV_STATE_DISABLED);
            ESP_LOGI(TAG, "Disabled button is now ENABLED");
        }
    }
}

/**
 * Disabled button click handler (should not fire when disabled)
 */
static void onDisabledClick(lv_event_t* e) {
    ESP_LOGI(TAG, "Disabled Button Clicked! (Should only work when enabled)");
    
    if (statusLabel) {
        statusLabel->setText("Disabled button is now active!");
        lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(255, 165, 0), 0);
    }
}

/**
 * Main test function
 */
extern "C" void test_button_control() {
    ESP_LOGI(TAG, "\n=== Button Control Example ===");
    ESP_LOGI(TAG, "Testing: LVButton with multiple styles and states\n");
    
    // Create container panel
    LVPanel* container = new LVPanel(nullptr);
    container->setSize(800, 500);
    lv_obj_center(container->obj());
    lv_obj_set_style_bg_color(container->obj(), lv_color_make(25, 25, 35), 0);
    lv_obj_set_style_bg_opa(container->obj(), LV_OPA_COVER, 0);
    lv_obj_set_style_radius(container->obj(), 15, 0);
    lv_obj_set_style_border_width(container->obj(), 2, 0);
    lv_obj_set_style_border_color(container->obj(), lv_color_make(80, 80, 100), 0);
    
    // Title
    LVLabel* titleLabel = new LVLabel(container);
    titleLabel->setText("Button Testing Panel");
    lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(titleLabel->obj(), lv_color_white(), 0);
    lv_obj_align(titleLabel->obj(), LV_ALIGN_TOP_MID, 0, 20);
    
    // ========== PRIMARY BUTTON ==========
    
    btnPrimary = new LVButton(container);
    btnPrimary->setText("PRIMARY");
    btnPrimary->setFont(&lv_font_montserrat_18);
    lv_obj_set_size(btnPrimary->obj(), 200, 60);
    lv_obj_align(btnPrimary->obj(), LV_ALIGN_TOP_LEFT, 50, 80);
    
    // Primary button styling (Blue theme)
    lv_obj_set_style_bg_color(btnPrimary->obj(), lv_color_make(0, 120, 215), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btnPrimary->obj(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(btnPrimary->obj(), 8, 0);
    lv_obj_set_style_shadow_width(btnPrimary->obj(), 10, 0);
    lv_obj_set_style_shadow_color(btnPrimary->obj(), lv_color_make(0, 0, 0), 0);
    
    // Pressed state
    lv_obj_set_style_bg_color(btnPrimary->obj(), lv_color_make(0, 90, 180), LV_STATE_PRESSED);
    
    lv_obj_add_event_cb(btnPrimary->obj(), onPrimaryClick, LV_EVENT_CLICKED, nullptr);
    
    printf("🔍 Primary Button: %p, size: %dx%d\n", 
           (void*)btnPrimary->obj(),
           lv_obj_get_width(btnPrimary->obj()),
           lv_obj_get_height(btnPrimary->obj()));
    
    // ========== SECONDARY BUTTON ==========
    
    btnSecondary = new LVButton(container);
    btnSecondary->setText("SECONDARY");
    btnSecondary->setFont(&lv_font_montserrat_18);
    lv_obj_set_size(btnSecondary->obj(), 200, 60);
    lv_obj_align(btnSecondary->obj(), LV_ALIGN_TOP_MID, 0, 80);
    
    // Secondary button styling (Gray theme)
    lv_obj_set_style_bg_color(btnSecondary->obj(), lv_color_make(100, 100, 110), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btnSecondary->obj(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(btnSecondary->obj(), 8, 0);
    lv_obj_set_style_border_width(btnSecondary->obj(), 2, 0);
    lv_obj_set_style_border_color(btnSecondary->obj(), lv_color_make(150, 150, 160), 0);
    
    // Pressed state
    lv_obj_set_style_bg_color(btnSecondary->obj(), lv_color_make(70, 70, 80), LV_STATE_PRESSED);
    
    lv_obj_add_event_cb(btnSecondary->obj(), onSecondaryClick, LV_EVENT_CLICKED, nullptr);
    
    printf("🔍 Secondary Button: %p, size: %dx%d\n", 
           (void*)btnSecondary->obj(),
           lv_obj_get_width(btnSecondary->obj()),
           lv_obj_get_height(btnSecondary->obj()));
    
    // ========== DANGER BUTTON ==========
    
    btnDanger = new LVButton(container);
    btnDanger->setText("DANGER");
    btnDanger->setFont(&lv_font_montserrat_18);
    lv_obj_set_size(btnDanger->obj(), 200, 60);
    lv_obj_align(btnDanger->obj(), LV_ALIGN_TOP_RIGHT, -50, 80);
    
    // Danger button styling (Red theme)
    lv_obj_set_style_bg_color(btnDanger->obj(), lv_color_make(220, 53, 69), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btnDanger->obj(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(btnDanger->obj(), 8, 0);
    lv_obj_set_style_shadow_width(btnDanger->obj(), 10, 0);
    lv_obj_set_style_shadow_color(btnDanger->obj(), lv_color_make(150, 0, 0), 0);
    
    // Pressed state
    lv_obj_set_style_bg_color(btnDanger->obj(), lv_color_make(180, 30, 50), LV_STATE_PRESSED);
    
    lv_obj_add_event_cb(btnDanger->obj(), onDangerClick, LV_EVENT_CLICKED, nullptr);
    
    printf("🔍 Danger Button: %p, size: %dx%d\n", 
           (void*)btnDanger->obj(),
           lv_obj_get_width(btnDanger->obj()),
           lv_obj_get_height(btnDanger->obj()));
    
    // ========== DISABLED BUTTON ==========
    
    btnDisabled = new LVButton(container);
    btnDisabled->setText("DISABLED (Click DANGER to toggle)");
    btnDisabled->setFont(&lv_font_montserrat_16);
    btnDisabled->setTextColor(lv_color_make(120, 120, 120));
    lv_obj_set_size(btnDisabled->obj(), 400, 60);
    lv_obj_align(btnDisabled->obj(), LV_ALIGN_TOP_MID, 0, 180);
    
    // Disabled button styling
    lv_obj_set_style_bg_color(btnDisabled->obj(), lv_color_make(60, 60, 70), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btnDisabled->obj(), LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_radius(btnDisabled->obj(), 8, 0);
    
    // Set disabled state
    lv_obj_add_state(btnDisabled->obj(), LV_STATE_DISABLED);
    
    lv_obj_add_event_cb(btnDisabled->obj(), onDisabledClick, LV_EVENT_CLICKED, nullptr);
    
    printf("🔍 Disabled Button: %p, size: %dx%d\n", 
           (void*)btnDisabled->obj(),
           lv_obj_get_width(btnDisabled->obj()),
           lv_obj_get_height(btnDisabled->obj()));
    
    // ========== STATUS LABEL ==========
    
    statusLabel = new LVLabel(container);
    statusLabel->setText("Click any button to see status");
    lv_obj_set_style_text_font(statusLabel->obj(), &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(200, 200, 210), 0);
    lv_obj_align(statusLabel->obj(), LV_ALIGN_CENTER, 0, 50);
    
    // ========== INSTRUCTIONS ==========
    
    LVLabel* instructionLabel = new LVLabel(container);
    instructionLabel->setText("Tap buttons to test events and styling");
    lv_obj_set_style_text_color(instructionLabel->obj(), lv_color_make(150, 150, 160), 0);
    lv_obj_align(instructionLabel->obj(), LV_ALIGN_BOTTOM_MID, 0, -20);
    
    ESP_LOGI(TAG, "✅ Button Control UI created");
    ESP_LOGI(TAG, "\n📊 Testing points:");
    ESP_LOGI(TAG, "   1. Primary button (Blue) - Click counter");
    ESP_LOGI(TAG, "   2. Secondary button (Gray) - Simple action");
    ESP_LOGI(TAG, "   3. Danger button (Red) - Toggles disabled button");
    ESP_LOGI(TAG, "   4. Disabled button - Only works when enabled");
    ESP_LOGI(TAG, "   5. Visual feedback on press/release");
    
    ESP_LOGI(TAG, "\n🧪 Manual Test Cases:");
    ESP_LOGI(TAG, "   TC1: Tap PRIMARY multiple times  → Counter increments");
    ESP_LOGI(TAG, "   TC2: Tap SECONDARY               → Status changes");
    ESP_LOGI(TAG, "   TC3: Tap DANGER                  → Toggles disabled state");
    ESP_LOGI(TAG, "   TC4: Tap DISABLED when disabled  → No response");
    ESP_LOGI(TAG, "   TC5: Enable then tap DISABLED    → Should respond");
    ESP_LOGI(TAG, "   TC6: Visual press feedback       → Colors change on press\n");
}
