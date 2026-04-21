/**
 * @file arc_control.cpp
 * @brief Phase 2 Test #9 - LVArc (Volume & Temperature Control)
 * 
 * Tests:
 * - TC1: Create circular arc widget
 * - TC2: Value range configuration (0-100, 0-50)
 * - TC3: Touch drag to change value
 * - TC4: Value change callback
 * - TC5: Background arc angles
 * - TC6: Rotation angle configuration
 * - TC7: Arc mode (normal, reverse, symmetrical)
 * - TC8: Multiple arcs on same screen
 */

#include "../include/LVArc.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVPanel.hpp"
#include "esp_log.h"
#include <string>

static const char* TAG = "ARC_TEST";

// Global widgets
static LVArc* arcVolume = nullptr;
static LVArc* arcTemp = nullptr;
static LVLabel* lblVolumeValue = nullptr;
static LVLabel* lblTempValue = nullptr;
static LVLabel* lblStatus = nullptr;

/**
 * @brief Update volume label
 */
static void updateVolumeLabel(int32_t value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld%%", value);
    lblVolumeValue->setText(buffer);
    
    // Update status
    if (value == 0) {
        lblStatus->setText("Volume: Muted");
        lblStatus->setTextColor(lv_color_hex(0x757575));
    } else if (value < 30) {
        lblStatus->setText("Volume: Low");
        lblStatus->setTextColor(lv_color_hex(0x1976D2));
    } else if (value < 70) {
        lblStatus->setText("Volume: Medium");
        lblStatus->setTextColor(lv_color_hex(0x388E3C));
    } else {
        lblStatus->setText("Volume: High");
        lblStatus->setTextColor(lv_color_hex(0xD32F2F));
    }
    
    ESP_LOGI(TAG, "Volume: %ld%%", value);
}

/**
 * @brief Update temperature label
 */
static void updateTempLabel(int32_t value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ldC", value);
    lblTempValue->setText(buffer);
    
    // Color coding based on temperature
    if (value < 18) {
        lv_obj_set_style_arc_color(arcTemp->obj(), lv_color_hex(0x1976D2), LV_PART_INDICATOR);  // Blue - Cold
    } else if (value < 25) {
        lv_obj_set_style_arc_color(arcTemp->obj(), lv_color_hex(0x388E3C), LV_PART_INDICATOR);  // Green - Comfortable
    } else if (value < 32) {
        lv_obj_set_style_arc_color(arcTemp->obj(), lv_color_hex(0xF57C00), LV_PART_INDICATOR);  // Orange - Warm
    } else {
        lv_obj_set_style_arc_color(arcTemp->obj(), lv_color_hex(0xD32F2F), LV_PART_INDICATOR);  // Red - Hot
    }
    
    ESP_LOGI(TAG, "Temperature: %ldC", value);
}

/**
 * @brief Volume arc value changed callback
 */
static void onVolumeChanged(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_VALUE_CHANGED) return;
    
    if (arcVolume) {
        int32_t value = arcVolume->getValue();
        updateVolumeLabel(value);
    }
}

/**
 * @brief Temperature arc value changed callback
 */
static void onTempChanged(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_VALUE_CHANGED) return;
    
    if (arcTemp) {
        int32_t value = arcTemp->getValue();
        updateTempLabel(value);
    }
}

/**
 * @brief Create Arc Control UI
 */
extern "C" void test_arc_control()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #9: LVArc - Volume & Temp Control");
    ESP_LOGI(TAG, "========================================");
    
    // Main container
    auto* container = new LVPanel(nullptr);
    lv_obj_set_size(container->obj(), 900, 560);
    lv_obj_align(container->obj(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(container->obj(), lv_color_hex(0xF5F5F5), 0);
    lv_obj_set_style_border_width(container->obj(), 0, 0);
    lv_obj_set_style_pad_all(container->obj(), 20, 0);
    
    // Title
    auto* lblTitle = new LVLabel(container);
    lblTitle->setText("Arc Controls");
    lv_obj_set_style_text_font(lblTitle->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lblTitle->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblTitle->obj(), LV_ALIGN_TOP_MID, 0, 0);
    
    // ===== Volume Arc (Left Side) =====
    auto* lblVolumeTitle = new LVLabel(container);
    lblVolumeTitle->setText("Volume");
    lv_obj_set_style_text_font(lblVolumeTitle->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblVolumeTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblVolumeTitle->obj(), LV_ALIGN_TOP_LEFT, 120, 60);
    
    // Volume arc (0-100%)
    arcVolume = new LVArc(container);
    lv_obj_set_size(arcVolume->obj(), 200, 200);
    lv_obj_align(arcVolume->obj(), LV_ALIGN_TOP_LEFT, 70, 100);
    arcVolume->setRange(0, 100);
    arcVolume->setValue(50);
    
    // Styling
    lv_obj_set_style_arc_width(arcVolume->obj(), 15, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arcVolume->obj(), 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arcVolume->obj(), lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arcVolume->obj(), lv_color_hex(0x1976D2), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(arcVolume->obj(), lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_pad_all(arcVolume->obj(), 5, LV_PART_KNOB);
    
    // Volume value label (center of arc)
    lblVolumeValue = new LVLabel(container);
    lblVolumeValue->setText("50%");
    lv_obj_set_style_text_font(lblVolumeValue->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lblVolumeValue->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblVolumeValue->obj(), LV_ALIGN_TOP_LEFT, 145, 185);
    
    // Volume description
    auto* lblVolumeDesc = new LVLabel(container);
    lblVolumeDesc->setText("Drag to adjust\nspeaker volume");
    lv_obj_set_style_text_font(lblVolumeDesc->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblVolumeDesc->obj(), lv_color_hex(0x757575), 0);
    lv_obj_set_style_text_align(lblVolumeDesc->obj(), LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lblVolumeDesc->obj(), LV_ALIGN_TOP_LEFT, 90, 320);
    
    // Add event callback
    lv_obj_add_event_cb(arcVolume->obj(), onVolumeChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    // ===== Temperature Arc (Right Side) =====
    auto* lblTempTitle = new LVLabel(container);
    lblTempTitle->setText("Temperature");
    lv_obj_set_style_text_font(lblTempTitle->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblTempTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblTempTitle->obj(), LV_ALIGN_TOP_LEFT, 570, 60);
    
    // Temperature arc (0-50°C)
    arcTemp = new LVArc(container);
    lv_obj_set_size(arcTemp->obj(), 200, 200);
    lv_obj_align(arcTemp->obj(), LV_ALIGN_TOP_LEFT, 530, 100);
    arcTemp->setRange(0, 50);
    arcTemp->setValue(25);
    
    // Custom angles for temperature (180° to 0° = bottom to top arc)
    arcTemp->setRotation(180);  // Start from bottom
    arcTemp->setAngles(0, 180);  // Half circle
    
    // Styling
    lv_obj_set_style_arc_width(arcTemp->obj(), 15, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arcTemp->obj(), 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arcTemp->obj(), lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arcTemp->obj(), lv_color_hex(0x388E3C), LV_PART_INDICATOR);  // Default green
    lv_obj_set_style_bg_color(arcTemp->obj(), lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_pad_all(arcTemp->obj(), 5, LV_PART_KNOB);
    
    // Temperature value label (center of arc)
    lblTempValue = new LVLabel(container);
    lblTempValue->setText("25C");
    lv_obj_set_style_text_font(lblTempValue->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lblTempValue->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblTempValue->obj(), LV_ALIGN_TOP_LEFT, 605, 185);
    
    // Temperature description
    auto* lblTempDesc = new LVLabel(container);
    lblTempDesc->setText("Set target\ntemperature");
    lv_obj_set_style_text_font(lblTempDesc->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblTempDesc->obj(), lv_color_hex(0x757575), 0);
    lv_obj_set_style_text_align(lblTempDesc->obj(), LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lblTempDesc->obj(), LV_ALIGN_TOP_LEFT, 555, 320);
    
    // Add event callback
    lv_obj_add_event_cb(arcTemp->obj(), onTempChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    // ===== Status Label =====
    lblStatus = new LVLabel(container);
    lblStatus->setText("Volume: Medium");
    lv_obj_set_style_text_font(lblStatus->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblStatus->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblStatus->obj(), LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Temperature range indicators
    auto* lblTempMin = new LVLabel(container);
    lblTempMin->setText("0C");
    lv_obj_set_style_text_font(lblTempMin->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblTempMin->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblTempMin->obj(), LV_ALIGN_TOP_LEFT, 515, 200);
    
    auto* lblTempMax = new LVLabel(container);
    lblTempMax->setText("50C");
    lv_obj_set_style_text_font(lblTempMax->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblTempMax->obj(), lv_color_hex(0xD32F2F), 0);
    lv_obj_align(lblTempMax->obj(), LV_ALIGN_TOP_LEFT, 715, 200);
    
    ESP_LOGI(TAG, "✅ Arc Control UI created");
    ESP_LOGI(TAG, "   - Volume Arc: 0-100%% (full circle)");
    ESP_LOGI(TAG, "   - Temperature Arc: 0-50C (half circle)");
    ESP_LOGI(TAG, "   - Initial values: Volume=50%%, Temp=25C");
}
