/**
 * @file switch_control.cpp
 * @brief Phase 2 Testing: LVSwitch + LVLabel Integration
 * 
 * Test Cases:
 * - LVSwitch creation and event handling
 * - LVLabel text updates based on switch state
 * - ON/OFF state management
 * - Touch interaction
 * - Style customization
 */

#include "lvgl.h"
#include "LVWidget.hpp"
#include "LVSwitch.hpp"
#include "LVLabel.hpp"
#include "LVPanel.hpp"
#include "esp_log.h"
#include <stdio.h>

static const char* TAG = "SWITCH_TEST";

// UI Components
static LVSwitch* wifiSwitch = nullptr;
static LVLabel* wifiStatusLabel = nullptr;
static LVLabel* wifiDescLabel = nullptr;

static LVSwitch* bluetoothSwitch = nullptr;
static LVLabel* bluetoothStatusLabel = nullptr;
static LVLabel* bluetoothDescLabel = nullptr;

/**
 * Event handler for WiFi switch
 */
static void onWiFiSwitchChanged(lv_event_t* e) {
    lv_obj_t* switch_obj = (lv_obj_t*)lv_event_get_target(e);
    bool state = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "WiFi Switch: %s", state ? "ON" : "OFF");
    
    if (wifiStatusLabel) {
        if (state) {
            wifiStatusLabel->setText("WiFi: ON");
            lv_obj_set_style_text_color(wifiStatusLabel->obj(), 
                                       lv_color_make(0, 200, 0), 0);
        } else {
            wifiStatusLabel->setText("WiFi: OFF");
            lv_obj_set_style_text_color(wifiStatusLabel->obj(), 
                                       lv_color_make(200, 0, 0), 0);
        }
    }
    
    if (wifiDescLabel) {
        if (state) {
            wifiDescLabel->setText("Connected to network");
        } else {
            wifiDescLabel->setText("Disconnected");
        }
    }
}

/**
 * Event handler for Bluetooth switch
 */
static void onBluetoothSwitchChanged(lv_event_t* e) {
    lv_obj_t* switch_obj = (lv_obj_t*)lv_event_get_target(e);
    bool state = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "Bluetooth Switch: %s", state ? "ON" : "OFF");
    
    if (bluetoothStatusLabel) {
        if (state) {
            bluetoothStatusLabel->setText("Bluetooth: ON");
            lv_obj_set_style_text_color(bluetoothStatusLabel->obj(), 
                                       lv_color_make(0, 100, 200), 0);
        } else {
            bluetoothStatusLabel->setText("Bluetooth: OFF");
            lv_obj_set_style_text_color(bluetoothStatusLabel->obj(), 
                                       lv_color_make(150, 150, 150), 0);
        }
    }
    
    if (bluetoothDescLabel) {
        if (state) {
            bluetoothDescLabel->setText("Ready to pair");
        } else {
            bluetoothDescLabel->setText("Disabled");
        }
    }
}

/**
 * Main test function
 */
extern "C" void test_switch_control() {
    ESP_LOGI(TAG, "\n=== Switch Control Example ===");
    ESP_LOGI(TAG, "Testing: LVSwitch + LVLabel integration\n");
    
    // Create container panel
    LVPanel* container = new LVPanel(nullptr);
    container->setSize(700, 400);
    lv_obj_center(container->obj());
    lv_obj_set_style_bg_color(container->obj(), lv_color_make(30, 30, 30), 0);
    lv_obj_set_style_bg_opa(container->obj(), LV_OPA_COVER, 0);
    lv_obj_set_style_radius(container->obj(), 20, 0);
    lv_obj_set_style_border_width(container->obj(), 2, 0);
    lv_obj_set_style_border_color(container->obj(), lv_color_make(100, 100, 100), 0);
    
    // Title
    LVLabel* titleLabel = new LVLabel(container);
    titleLabel->setText("Settings Panel");
    lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(titleLabel->obj(), lv_color_white(), 0);
    lv_obj_align(titleLabel->obj(), LV_ALIGN_TOP_MID, 0, 20);
    
    // ========== WiFi Control Section ==========
    
    // WiFi switch
    wifiSwitch = new LVSwitch(container);
    lv_obj_set_size(wifiSwitch->obj(), 80, 40);
    lv_obj_align(wifiSwitch->obj(), LV_ALIGN_TOP_LEFT, 50, 80);
    
    // Set initial state to ON
    lv_obj_add_state(wifiSwitch->obj(), LV_STATE_CHECKED);
    
    // Style WiFi switch
    lv_obj_set_style_bg_color(wifiSwitch->obj(), lv_color_make(100, 100, 100), LV_PART_MAIN);
    lv_obj_set_style_bg_color(wifiSwitch->obj(), lv_color_make(0, 150, 0), LV_PART_INDICATOR);
    
    printf("🔍 WiFi Switch created: %p, size: %dx%d\n", 
           (void*)wifiSwitch->obj(),
           lv_obj_get_width(wifiSwitch->obj()),
           lv_obj_get_height(wifiSwitch->obj()));
    
    // WiFi status label
    wifiStatusLabel = new LVLabel(container);
    wifiStatusLabel->setText("WiFi: ON");
    lv_obj_set_style_text_font(wifiStatusLabel->obj(), &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(wifiStatusLabel->obj(), lv_color_make(0, 200, 0), 0);
    lv_obj_align(wifiStatusLabel->obj(), LV_ALIGN_TOP_LEFT, 150, 85);
    
    // WiFi description label
    wifiDescLabel = new LVLabel(container);
    wifiDescLabel->setText("Connected to network");
    lv_obj_set_style_text_font(wifiDescLabel->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(wifiDescLabel->obj(), lv_color_make(150, 150, 150), 0);
    lv_obj_align(wifiDescLabel->obj(), LV_ALIGN_TOP_LEFT, 150, 115);
    
    // Add WiFi event callback
    lv_obj_add_event_cb(wifiSwitch->obj(), onWiFiSwitchChanged, 
                        LV_EVENT_VALUE_CHANGED, nullptr);
    
    // ========== Bluetooth Control Section ==========
    
    // Bluetooth switch
    bluetoothSwitch = new LVSwitch(container);
    lv_obj_set_size(bluetoothSwitch->obj(), 80, 40);
    lv_obj_align(bluetoothSwitch->obj(), LV_ALIGN_TOP_LEFT, 50, 180);
    
    // Set initial state to OFF
    // (already OFF by default)
    
    // Style Bluetooth switch
    lv_obj_set_style_bg_color(bluetoothSwitch->obj(), lv_color_make(100, 100, 100), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bluetoothSwitch->obj(), lv_color_make(0, 100, 200), LV_PART_INDICATOR);
    
    printf("🔍 Bluetooth Switch created: %p, size: %dx%d\n", 
           (void*)bluetoothSwitch->obj(),
           lv_obj_get_width(bluetoothSwitch->obj()),
           lv_obj_get_height(bluetoothSwitch->obj()));
    
    // Bluetooth status label
    bluetoothStatusLabel = new LVLabel(container);
    bluetoothStatusLabel->setText("Bluetooth: OFF");
    lv_obj_set_style_text_font(bluetoothStatusLabel->obj(), &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(bluetoothStatusLabel->obj(), lv_color_make(150, 150, 150), 0);
    lv_obj_align(bluetoothStatusLabel->obj(), LV_ALIGN_TOP_LEFT, 150, 185);
    
    // Bluetooth description label
    bluetoothDescLabel = new LVLabel(container);
    bluetoothDescLabel->setText("Disabled");
    lv_obj_set_style_text_font(bluetoothDescLabel->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bluetoothDescLabel->obj(), lv_color_make(150, 150, 150), 0);
    lv_obj_align(bluetoothDescLabel->obj(), LV_ALIGN_TOP_LEFT, 150, 215);
    
    // Add Bluetooth event callback
    lv_obj_add_event_cb(bluetoothSwitch->obj(), onBluetoothSwitchChanged, 
                        LV_EVENT_VALUE_CHANGED, nullptr);
    
    // ========== Test Instructions ==========
    
    LVLabel* instructionLabel = new LVLabel(container);
    instructionLabel->setText("Tap switches to toggle ON/OFF");
    lv_obj_set_style_text_color(instructionLabel->obj(), lv_color_make(200, 200, 200), 0);
    lv_obj_align(instructionLabel->obj(), LV_ALIGN_BOTTOM_MID, 0, -20);
    
    ESP_LOGI(TAG, "✅ Switch Control UI created");
    ESP_LOGI(TAG, "\n📊 Testing points:");
    ESP_LOGI(TAG, "   1. WiFi switch starts ON (green)");
    ESP_LOGI(TAG, "   2. Bluetooth switch starts OFF (gray)");
    ESP_LOGI(TAG, "   3. Labels update when switches toggle");
    ESP_LOGI(TAG, "   4. Colors change based on state");
    ESP_LOGI(TAG, "   5. Event callbacks fire correctly");
    
    ESP_LOGI(TAG, "\n🧪 Manual Test Cases:");
    ESP_LOGI(TAG, "   TC1: Tap WiFi switch      → Should turn OFF (red text)");
    ESP_LOGI(TAG, "   TC2: Tap WiFi again       → Should turn ON (green text)");
    ESP_LOGI(TAG, "   TC3: Tap Bluetooth switch → Should turn ON (blue text)");
    ESP_LOGI(TAG, "   TC4: Tap Bluetooth again  → Should turn OFF (gray text)");
    ESP_LOGI(TAG, "   TC5: Toggle rapidly       → UI should remain responsive");
    ESP_LOGI(TAG, "   TC6: Check description    → Text should match state\n");
}
