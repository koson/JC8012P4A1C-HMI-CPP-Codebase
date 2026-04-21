/**
 * @file brightness_control.cpp
 * @brief Brightness Control Example - Tests LVSlider, LVPanel, LVLabel
 * 
 * This example demonstrates:
 * - LVSlider value change callback
 * - LVPanel background color manipulation
 * - LVLabel dynamic text update
 * - Real-time UI updates
 * 
 * Expected behavior:
 * - Slider range: 0-100
 * - Panel brightness changes from dark (0) to bright (100)
 * - Label shows percentage value
 * 
 * Phase 2 Testing Objective:
 * - Verify LVSlider callback works
 * - Verify LVPanel color change
 * - Verify LVLabel setText() updates display
 * - Check for memory leaks
 * - Check for smooth operation
 */

#include "LVSlider.hpp"
#include "LVPanel.hpp"
#include "LVLabel.hpp"
#include <cstdio>

// Global widgets (will be created in setup)
static LVPanel* brightnessPanel = nullptr;
static LVLabel* valueLabel = nullptr;
static LVSlider* brightnessSlider = nullptr;

/**
 * Slider value changed callback
 * Updates panel brightness and label text
 */
void onBrightnessChanged(lv_event_t* e) {
    lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    if (!slider) return;
    
    // Get current slider value (0-100)
    int32_t value = lv_slider_get_value(slider);
    
    // Update label text
    char buf[32];
    snprintf(buf, sizeof(buf), "Brightness: %ld%%", value);
    valueLabel->setText(buf);
    
    // Calculate color based on brightness
    // 0 = dark gray (0x303030), 100 = light gray (0xF0F0F0)
    uint8_t brightness = (value * 192) / 100 + 48;  // Map 0-100 to 48-240
    lv_color_t color = lv_color_make(brightness, brightness, brightness);
    
    // Update panel background color
    lv_obj_set_style_bg_color(brightnessPanel->obj(), color, LV_PART_MAIN);
    
    // Debug output
    printf("Brightness changed: %ld%% (RGB: %d,%d,%d)\n", 
           value, brightness, brightness, brightness);
}

/**
 * Setup brightness control example
 */
extern "C" void test_brightness_control() {
    printf("\n=== Brightness Control Example ===\n");
    printf("Testing: LVSlider, LVPanel, LVLabel\n\n");
    
    // Create title label (heap allocated to persist)
    LVLabel* titleLabel = new LVLabel(nullptr);
    titleLabel->setText("Brightness Control Demo");
    titleLabel->setSize(LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(titleLabel->obj(), LV_ALIGN_TOP_MID, 0, 20);
    
    // Set title style
    lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(titleLabel->obj(), LV_TEXT_ALIGN_CENTER, 0);
    
    // Create brightness panel (preview area)
    brightnessPanel = new LVPanel(nullptr);
    brightnessPanel->setSize(400, 200);
    lv_obj_align(brightnessPanel->obj(), LV_ALIGN_CENTER, 0, -50);
    
    // Set initial panel style
    lv_obj_set_style_radius(brightnessPanel->obj(), 15, LV_PART_MAIN);
    lv_obj_set_style_bg_color(brightnessPanel->obj(), 
                               lv_color_make(128, 128, 128), LV_PART_MAIN);
    lv_obj_set_style_border_width(brightnessPanel->obj(), 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(brightnessPanel->obj(), 
                                   lv_color_white(), LV_PART_MAIN);
    
    // Create value label (inside panel)
    valueLabel = new LVLabel(brightnessPanel);
    valueLabel->setText("Brightness: 50%");
    valueLabel->setSize(LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(valueLabel->obj(), LV_ALIGN_CENTER, 0, 0);
    
    // Set label style
    lv_obj_set_style_text_font(valueLabel->obj(), &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(valueLabel->obj(), lv_color_white(), 0);
    
    // Create brightness slider using wrapper (FIXED)
    brightnessSlider = new LVSlider(nullptr);
    printf("🔍 Slider object created: %p\n", (void*)brightnessSlider->obj());
    
    // Set size
    lv_obj_set_width(brightnessSlider->obj(), 500);
    lv_obj_set_height(brightnessSlider->obj(), 40);
    
    // Then align
    lv_obj_align(brightnessSlider->obj(), LV_ALIGN_BOTTOM_MID, 0, -100);
    
    // Configure slider
    brightnessSlider->setRange(0, 100);
    brightnessSlider->setValue(50, LV_ANIM_OFF);
    
    // Bright colors for visibility
    lv_obj_set_style_bg_color(brightnessSlider->obj(), 
                               lv_color_make(255, 0, 0), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(brightnessSlider->obj(), LV_OPA_COVER, LV_PART_MAIN);
    
    lv_obj_set_style_bg_color(brightnessSlider->obj(), 
                               lv_color_make(0, 255, 0), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(brightnessSlider->obj(), LV_OPA_COVER, LV_PART_INDICATOR);
    
    lv_obj_set_style_bg_color(brightnessSlider->obj(), 
                               lv_color_make(255, 255, 0), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(brightnessSlider->obj(), LV_OPA_COVER, LV_PART_KNOB);
    
    printf("🔍 Slider size: %dx%d, pos: %d,%d\n", 
           lv_obj_get_width(brightnessSlider->obj()), 
           lv_obj_get_height(brightnessSlider->obj()),
           lv_obj_get_x(brightnessSlider->obj()),
           lv_obj_get_y(brightnessSlider->obj()));
    
    // Add event callback
    lv_obj_add_event_cb(brightnessSlider->obj(), onBrightnessChanged, 
                        LV_EVENT_VALUE_CHANGED, nullptr);
    
    // Create instruction label (heap allocated to persist)
    LVLabel* instructionLabel = new LVLabel(nullptr);
    instructionLabel->setText("Drag the YELLOW SLIDER to adjust brightness");
    instructionLabel->setSize(LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(instructionLabel->obj(), LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_align(instructionLabel->obj(), LV_TEXT_ALIGN_CENTER, 0);
    
    printf("✅ Brightness Control UI created\n");
    printf("📊 Testing points:\n");
    printf("   1. Slider responds to touch\n");
    printf("   2. Panel changes brightness smoothly\n");
    printf("   3. Label updates in real-time\n");
    printf("   4. Value range 0-100 works correctly\n");
    printf("   5. No memory leaks during interaction\n\n");
    
    // Test Points Documentation
    printf("🧪 Manual Test Cases:\n");
    printf("   TC1: Move slider to 0   → Panel should be dark gray\n");
    printf("   TC2: Move slider to 100 → Panel should be light gray\n");
    printf("   TC3: Move slider to 50  → Panel should be medium gray\n");
    printf("   TC4: Rapid movements    → UI should remain responsive\n");
    printf("   TC5: Touch and hold     → Should update continuously\n\n");
}

/**
 * Cleanup function (call on exit)
 */
void cleanup_brightness_control() {
    delete brightnessSlider;
    delete valueLabel;
    delete brightnessPanel;
    
    printf("✅ Brightness Control cleanup complete\n");
}
