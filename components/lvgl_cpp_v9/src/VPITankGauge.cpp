/**
 * @file VPITankGauge.cpp
 * @brief VPI Tank Pressure Gauge Implementation
 */

#include "VPITankGauge.h"
#include "esp_log.h"
#include <cstdio>

static const char *TAG = "VPITankGauge";

// External needle image declaration
extern "C" {
    LV_IMAGE_DECLARE(img_pressure_needle);
}

VPITankGauge::VPITankGauge(lv_obj_t* parent, const lv_image_dsc_t* needle_img)
    : gauge_(nullptr)
    , needle_(nullptr)
    , pressure_label_(nullptr)
    , current_pressure_(0.0f)
{
    ESP_LOGI(TAG, "Creating VPITankGauge...");
    
    // Create scale widget (gauge)
    gauge_ = lv_scale_create(parent);
    lv_obj_set_size(gauge_, 340, 340);
    
    // Configure as round scale
    lv_scale_set_mode(gauge_, LV_SCALE_MODE_ROUND_INNER);
    
    // Apply styles
    applyStyles();
    
    // Configure scale range and ticks (0-100%)
    lv_scale_set_label_show(gauge_, true);
    lv_scale_set_total_tick_count(gauge_, 21);  // 21 ticks for 0-100 (every 5%)
    lv_scale_set_major_tick_every(gauge_, 2);   // Major tick every 10%
    
    // Set range and angle
    lv_scale_set_range(gauge_, 0, 100);     // 0-100%
    lv_scale_set_angle_range(gauge_, 270);  // 270° arc
    lv_scale_set_rotation(gauge_, 135);     // Start at 135°
    
    ESP_LOGI(TAG, "Creating image needle with image @ %p", needle_img);
    
    // Create image needle
    needle_ = lv_image_create(gauge_);
    lv_image_set_src(needle_, needle_img);
    
    // Scale needle to 150% (256 = 100%, 384 = 150%)
    lv_image_set_scale(needle_, 384);
    
    // Set pivot point (rotation center)
    lv_image_set_pivot(needle_, 4, 4);
    
    // Recolor needle to red (since image is grayscale)
    lv_obj_set_style_img_recolor(needle_, lv_color_hex(0xBBBBBB), 0);
    lv_obj_set_style_img_recolor_opa(needle_, 255, 0);  // Full opacity
    
    // Position needle at gauge center
    lv_obj_align(needle_, LV_ALIGN_CENTER, 45, 0);
    
    ESP_LOGI(TAG, "Setting initial needle value to 0");
    
    // Initialize needle at 0%
    lv_scale_set_image_needle_value(gauge_, needle_, 0);
    
    // Create digital readout
    createDigitalReadout();
    
    // Move needle to foreground (above label)
    lv_obj_move_foreground(needle_);
    
    ESP_LOGI(TAG, "VPITankGauge created successfully");
}

VPITankGauge::~VPITankGauge()
{
    // LVGL will handle cleanup when parent is deleted
}

void VPITankGauge::updatePressure(float percentage)
{
    ESP_LOGI(TAG, "updatePressure called: %.2f%%", percentage);
    
    // Clamp value to 0-100%
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;
    
    current_pressure_ = percentage;
    
    // Check if needle is valid
    if (needle_ == nullptr) {
        ESP_LOGE(TAG, "needle_ is NULL!");
        return;
    }
    if (gauge_ == nullptr) {
        ESP_LOGE(TAG, "gauge_ is NULL!");
        return;
    }
    
    // Update needle rotation
    ESP_LOGI(TAG, "Setting needle value to %d", (int32_t)percentage);
    lv_scale_set_image_needle_value(gauge_, needle_, (int32_t)percentage);
    
    // Update digital readout
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f%%", percentage);
    lv_label_set_text(pressure_label_, buf);
    ESP_LOGI(TAG, "Pressure updated successfully");
}

void VPITankGauge::setPosition(int x, int y)
{
    lv_obj_set_pos(gauge_, x, y);
}

void VPITankGauge::setSize(int width)
{
    lv_obj_set_size(gauge_, width, width);  // Keep it square (circular)
}

void VPITankGauge::applyStyles()
{
    // Background style
    lv_obj_set_style_bg_opa(gauge_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(gauge_, lv_color_hex(0x2a2a2a), 0);
    lv_obj_set_style_radius(gauge_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gauge_, true, 0);
    lv_obj_set_style_border_width(gauge_, 4, 0);
    lv_obj_set_style_border_color(gauge_, lv_color_hex(0x00FF00), 0);  // Green border
    
    // Tick styling
    lv_obj_set_style_length(gauge_, 8, LV_PART_ITEMS);       // Minor tick length
    lv_obj_set_style_length(gauge_, 15, LV_PART_INDICATOR);  // Major tick length
    lv_obj_set_style_line_width(gauge_, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_width(gauge_, 3, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(gauge_, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);      // White minor ticks
    lv_obj_set_style_line_color(gauge_, lv_color_hex(0x00FF00), LV_PART_INDICATOR);  // Green major ticks
    
    // Label styling (0-100 numbers)
    lv_obj_set_style_text_color(gauge_, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);  // White text
    lv_obj_set_style_text_font(gauge_, &lv_font_montserrat_20, LV_PART_INDICATOR);   // Large font
}

void VPITankGauge::createDigitalReadout()
{
    // Get parent of gauge (tank panel)
    lv_obj_t* parent = lv_obj_get_parent(gauge_);
    
    // Create digital pressure label below gauge
    pressure_label_ = lv_label_create(parent);
    lv_label_set_text(pressure_label_, "0.0%");
    lv_obj_set_style_text_color(pressure_label_, lv_color_hex(0x00FF00), 0);  // Green
    lv_obj_set_style_text_font(pressure_label_, &lv_font_montserrat_32, 0);   // Large font
    lv_obj_set_style_bg_color(pressure_label_, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(pressure_label_, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(pressure_label_, 5, 0);
    lv_obj_set_width(pressure_label_, 150);
    lv_obj_set_style_text_align(pressure_label_, LV_TEXT_ALIGN_CENTER, 0);
    
    // Position below gauge (outside, not inside)
    lv_obj_align_to(pressure_label_, gauge_, LV_ALIGN_OUT_BOTTOM_MID, 50, 30);
}
