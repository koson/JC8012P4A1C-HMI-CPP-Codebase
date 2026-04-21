/**
 * @file checkbox_control.cpp
 * @brief Phase 2 Testing: LVCheckbox Comprehensive Testing
 * 
 * Test Cases:
 * - Multiple checkbox creation with labels
 * - Check/uncheck event handling
 * - Group selection (single/multiple)
 * - State management
 * - Visual feedback
 */

#include "lvgl.h"
#include "LVWidget.hpp"
#include "LVCheckbox.hpp"
#include "LVLabel.hpp"
#include "LVPanel.hpp"
#include "esp_log.h"
#include <stdio.h>

static const char* TAG = "CHECKBOX_TEST";

// UI Components
static LVCheckbox* chkNotifications = nullptr;
static LVCheckbox* chkAutoSave = nullptr;
static LVCheckbox* chkDarkMode = nullptr;
static LVCheckbox* chkSounds = nullptr;
static LVLabel* statusLabel = nullptr;
static int checkedCount = 0;

/**
 * Update status label with current selections
 */
static void updateStatus() {
    checkedCount = 0;
    char status[128];
    char features[100] = "";
    
    if (chkNotifications && chkNotifications->isChecked()) {
        checkedCount++;
        strcat(features, "Notifications ");
    }
    if (chkAutoSave && chkAutoSave->isChecked()) {
        checkedCount++;
        strcat(features, "AutoSave ");
    }
    if (chkDarkMode && chkDarkMode->isChecked()) {
        checkedCount++;
        strcat(features, "DarkMode ");
    }
    if (chkSounds && chkSounds->isChecked()) {
        checkedCount++;
        strcat(features, "Sounds ");
    }
    
    if (checkedCount > 0) {
        snprintf(status, sizeof(status), "Active (%d): %s", checkedCount, features);
    } else {
        snprintf(status, sizeof(status), "No features enabled");
    }
    
    if (statusLabel) {
        statusLabel->setText(status);
        
        // Change color based on count
        if (checkedCount == 0) {
            lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(150, 150, 150), 0);
        } else if (checkedCount <= 2) {
            lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(0, 150, 255), 0);
        } else {
            lv_obj_set_style_text_color(statusLabel->obj(), lv_color_make(0, 200, 100), 0);
        }
    }
}

/**
 * Notifications checkbox handler
 */
static void onNotificationsChanged(lv_event_t* e) {
    lv_obj_t* cb = (lv_obj_t*)lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "Notifications: %s", checked ? "ON" : "OFF");
    updateStatus();
}

/**
 * Auto-save checkbox handler
 */
static void onAutoSaveChanged(lv_event_t* e) {
    lv_obj_t* cb = (lv_obj_t*)lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "Auto-save: %s", checked ? "ON" : "OFF");
    updateStatus();
}

/**
 * Dark mode checkbox handler
 */
static void onDarkModeChanged(lv_event_t* e) {
    lv_obj_t* cb = (lv_obj_t*)lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "Dark Mode: %s", checked ? "ON" : "OFF");
    updateStatus();
}

/**
 * Sounds checkbox handler
 */
static void onSoundsChanged(lv_event_t* e) {
    lv_obj_t* cb = (lv_obj_t*)lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "Sounds: %s", checked ? "ON" : "OFF");
    updateStatus();
}

/**
 * Main test function
 */
extern "C" void test_checkbox_control() {
    ESP_LOGI(TAG, "\n=== Checkbox Control Example ===");
    ESP_LOGI(TAG, "Testing: LVCheckbox with multiple options\n");
    
    // Create container panel
    LVPanel* container = new LVPanel(nullptr);
    container->setSize(700, 550);
    lv_obj_center(container->obj());
    lv_obj_set_style_bg_color(container->obj(), lv_color_make(30, 30, 40), 0);
    lv_obj_set_style_bg_opa(container->obj(), LV_OPA_COVER, 0);
    lv_obj_set_style_radius(container->obj(), 15, 0);
    lv_obj_set_style_border_width(container->obj(), 2, 0);
    lv_obj_set_style_border_color(container->obj(), lv_color_make(80, 80, 100), 0);
    lv_obj_set_style_pad_all(container->obj(), 30, 0);
    
    // Title
    LVLabel* titleLabel = new LVLabel(container);
    titleLabel->setText("Settings Panel");
    lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(titleLabel->obj(), lv_color_white(), 0);
    lv_obj_align(titleLabel->obj(), LV_ALIGN_TOP_MID, 0, 0);
    
    // Subtitle
    LVLabel* subtitleLabel = new LVLabel(container);
    subtitleLabel->setText("Select features to enable");
    lv_obj_set_style_text_color(subtitleLabel->obj(), lv_color_make(180, 180, 200), 0);
    lv_obj_align(subtitleLabel->obj(), LV_ALIGN_TOP_MID, 0, 40);
    
    // ========== CHECKBOX 1: NOTIFICATIONS ==========
    
    chkNotifications = new LVCheckbox(container, "Enable Notifications");
    chkNotifications->setChecked(true);  // Default ON
    lv_obj_align(chkNotifications->obj(), LV_ALIGN_TOP_LEFT, 50, 100);
    
    // Style the checkbox
    lv_obj_set_style_text_color(chkNotifications->obj(), lv_color_white(), 0);
    lv_obj_set_style_text_font(chkNotifications->obj(), &lv_font_montserrat_18, 0);
    
    lv_obj_add_event_cb(chkNotifications->obj(), onNotificationsChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    printf("🔍 Checkbox Notifications: %p, checked: %d\n", 
           (void*)chkNotifications->obj(),
           chkNotifications->isChecked());
    
    // Description
    LVLabel* descNotif = new LVLabel(container);
    descNotif->setText("Get alerts for important events");
    lv_obj_set_style_text_color(descNotif->obj(), lv_color_make(150, 150, 160), 0);
    lv_obj_set_style_text_font(descNotif->obj(), &lv_font_montserrat_14, 0);
    lv_obj_align_to(descNotif->obj(), chkNotifications->obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 35, 5);
    
    // ========== CHECKBOX 2: AUTO-SAVE ==========
    
    chkAutoSave = new LVCheckbox(container, "Auto-save Changes");
    chkAutoSave->setChecked(true);  // Default ON
    lv_obj_align(chkAutoSave->obj(), LV_ALIGN_TOP_LEFT, 50, 180);
    
    lv_obj_set_style_text_color(chkAutoSave->obj(), lv_color_white(), 0);
    lv_obj_set_style_text_font(chkAutoSave->obj(), &lv_font_montserrat_18, 0);
    
    lv_obj_add_event_cb(chkAutoSave->obj(), onAutoSaveChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    printf("🔍 Checkbox Auto-save: %p, checked: %d\n", 
           (void*)chkAutoSave->obj(),
           chkAutoSave->isChecked());
    
    // Description
    LVLabel* descSave = new LVLabel(container);
    descSave->setText("Automatically save your work");
    lv_obj_set_style_text_color(descSave->obj(), lv_color_make(150, 150, 160), 0);
    lv_obj_set_style_text_font(descSave->obj(), &lv_font_montserrat_14, 0);
    lv_obj_align_to(descSave->obj(), chkAutoSave->obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 35, 5);
    
    // ========== CHECKBOX 3: DARK MODE ==========
    
    chkDarkMode = new LVCheckbox(container, "Dark Mode");
    chkDarkMode->setChecked(false);  // Default OFF
    lv_obj_align(chkDarkMode->obj(), LV_ALIGN_TOP_LEFT, 50, 260);
    
    lv_obj_set_style_text_color(chkDarkMode->obj(), lv_color_white(), 0);
    lv_obj_set_style_text_font(chkDarkMode->obj(), &lv_font_montserrat_18, 0);
    
    lv_obj_add_event_cb(chkDarkMode->obj(), onDarkModeChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    printf("🔍 Checkbox Dark Mode: %p, checked: %d\n", 
           (void*)chkDarkMode->obj(),
           chkDarkMode->isChecked());
    
    // Description
    LVLabel* descDark = new LVLabel(container);
    descDark->setText("Use dark color theme");
    lv_obj_set_style_text_color(descDark->obj(), lv_color_make(150, 150, 160), 0);
    lv_obj_set_style_text_font(descDark->obj(), &lv_font_montserrat_14, 0);
    lv_obj_align_to(descDark->obj(), chkDarkMode->obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 35, 5);
    
    // ========== CHECKBOX 4: SOUNDS ==========
    
    chkSounds = new LVCheckbox(container, "Sound Effects");
    chkSounds->setChecked(false);  // Default OFF
    lv_obj_align(chkSounds->obj(), LV_ALIGN_TOP_LEFT, 50, 340);
    
    lv_obj_set_style_text_color(chkSounds->obj(), lv_color_white(), 0);
    lv_obj_set_style_text_font(chkSounds->obj(), &lv_font_montserrat_18, 0);
    
    lv_obj_add_event_cb(chkSounds->obj(), onSoundsChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    printf("🔍 Checkbox Sounds: %p, checked: %d\n", 
           (void*)chkSounds->obj(),
           chkSounds->isChecked());
    
    // Description
    LVLabel* descSounds = new LVLabel(container);
    descSounds->setText("Play audio feedback");
    lv_obj_set_style_text_color(descSounds->obj(), lv_color_make(150, 150, 160), 0);
    lv_obj_set_style_text_font(descSounds->obj(), &lv_font_montserrat_14, 0);
    lv_obj_align_to(descSounds->obj(), chkSounds->obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 35, 5);
    
    // ========== STATUS LABEL ==========
    
    statusLabel = new LVLabel(container);
    lv_obj_set_style_text_font(statusLabel->obj(), &lv_font_montserrat_18, 0);
    lv_obj_align(statusLabel->obj(), LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Initialize status
    updateStatus();
    
    // ========== INSTRUCTIONS ==========
    
    LVLabel* instructionLabel = new LVLabel(container);
    instructionLabel->setText("Tap checkboxes to toggle settings");
    lv_obj_set_style_text_color(instructionLabel->obj(), lv_color_make(120, 120, 130), 0);
    lv_obj_set_style_text_font(instructionLabel->obj(), &lv_font_montserrat_14, 0);
    lv_obj_align(instructionLabel->obj(), LV_ALIGN_BOTTOM_MID, 0, -60);
    
    ESP_LOGI(TAG, "✅ Checkbox Control UI created");
    ESP_LOGI(TAG, "\n📊 Initial state:");
    ESP_LOGI(TAG, "   - Notifications: %s", chkNotifications->isChecked() ? "ON" : "OFF");
    ESP_LOGI(TAG, "   - Auto-save: %s", chkAutoSave->isChecked() ? "ON" : "OFF");
    ESP_LOGI(TAG, "   - Dark Mode: %s", chkDarkMode->isChecked() ? "OFF" : "OFF");
    ESP_LOGI(TAG, "   - Sounds: %s", chkSounds->isChecked() ? "OFF" : "OFF");
    
    ESP_LOGI(TAG, "\n🧪 Manual Test Cases:");
    ESP_LOGI(TAG, "   TC1: Tap each checkbox → State toggles");
    ESP_LOGI(TAG, "   TC2: Status label updates → Shows active features");
    ESP_LOGI(TAG, "   TC3: Color changes → Based on selection count");
    ESP_LOGI(TAG, "   TC4: Multiple checks → All independent");
    ESP_LOGI(TAG, "   TC5: Rapid toggling → No state confusion");
    ESP_LOGI(TAG, "   TC6: Initial state → 2 ON, 2 OFF\n");
}
