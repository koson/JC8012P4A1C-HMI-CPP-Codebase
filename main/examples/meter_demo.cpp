/**
 * @file meter_demo.cpp
 * @brief Phase 2 Test #16: LVMeter - Gauge/Scale Displays
 * 
 * Test Cases:
 * - TC1: Round meter (speedometer style)
 * - TC2: Horizontal scale (thermometer style)
 * - TC3: Vertical scale (volume level)
 * - TC4: Custom tick configuration
 * - TC5: Different angle ranges
 * - TC6: Multiple meters with different modes
 * - TC7: Color-coded ranges
 * - TC8: Interactive value control
 */

#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVMeter.hpp"
#include "../include/LVSlider.hpp"
#include "../include/LVButton.hpp"
#include "esp_log.h"

static const char* TAG = "METER_TEST";

// ==================== Speedometer Meter Panel ====================
class SpeedometerPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVMeter* speedometer;
    LVSlider* speedSlider;
    LVLabel* speedLabel;
    
    int32_t currentSpeed = 0;

public:
    SpeedometerPanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 560);
        container->setPos(20, 20);
        container->setBackgroundColor(lv_color_hex(0x1A1A2E));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x16213E));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 20, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Speedometer");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Speedometer meter (0-200 km/h)
        speedometer = new LVMeter(container);
        speedometer->setSize(320, 320);
        speedometer->setAlign(LV_ALIGN_TOP_MID);
        speedometer->setPos(0, 50);
        speedometer->setMode(LV_SCALE_MODE_ROUND_INNER);
        speedometer->setRange(0, 200);
        speedometer->setAngleRange(270);  // Span 270° from NW to SW
        speedometer->setRotation(135);
        speedometer->setTotalTickCount(21);  // 0, 10, 20, ... 200
        speedometer->setMajorTickEvery(5);   // Every 5 ticks = 50 km/h
        
        // Style the meter
        lv_obj_set_style_bg_color(speedometer->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_border_width(speedometer->obj(), 3, LV_PART_MAIN);
        lv_obj_set_style_border_color(speedometer->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_radius(speedometer->obj(), LV_RADIUS_CIRCLE, LV_PART_MAIN);
        
        // Tick styling
        lv_obj_set_style_length(speedometer->obj(), 15, LV_PART_INDICATOR);  // Major ticks
        lv_obj_set_style_line_width(speedometer->obj(), 3, LV_PART_INDICATOR);
        lv_obj_set_style_line_color(speedometer->obj(), lv_color_hex(0xE74C3C), LV_PART_INDICATOR);
        
        lv_obj_set_style_length(speedometer->obj(), 8, LV_PART_ITEMS);  // Minor ticks
        lv_obj_set_style_line_width(speedometer->obj(), 2, LV_PART_ITEMS);
        lv_obj_set_style_line_color(speedometer->obj(), lv_color_hex(0xBDC3C7), LV_PART_ITEMS);
        
        // Text styling for tick labels
        lv_obj_set_style_text_color(speedometer->obj(), lv_color_hex(0xECF0F1), LV_PART_INDICATOR);
        lv_obj_set_style_text_font(speedometer->obj(), &lv_font_montserrat_14, LV_PART_INDICATOR);
        
        // Enable needle indicator (red pointer showing current speed)
        speedometer->enableLineNeedle(100, 4, lv_color_hex(0xE74C3C));
        
        // Speed label
        speedLabel = new LVLabel(container);
        speedLabel->setText("0 km/h");
        speedLabel->setTextColor(lv_color_hex(0x3498DB));
        speedLabel->setFont(&lv_font_montserrat_24);
        speedLabel->setAlign(LV_ALIGN_CENTER);
        speedLabel->setPos(0, 20);
        
        // Speed slider
        speedSlider = new LVSlider(container);
        speedSlider->setSize(380, 20);
        speedSlider->setAlign(LV_ALIGN_BOTTOM_MID);
        speedSlider->setPos(0, -50);
        speedSlider->setRange(0, 200);
        speedSlider->setValue(0);
        
        // Slider styling
        lv_obj_set_style_bg_color(speedSlider->obj(), lv_color_hex(0x3498DB), LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(speedSlider->obj(), lv_color_hex(0x34495E), LV_PART_MAIN);
        
        speedSlider->onChange([this](int32_t value) {
            currentSpeed = value;
            speedometer->setValue(value);
            
            // Calculate angle for debugging
            int32_t range = 200 - 0;
            int32_t angle = -((135 * 10) - ((value * 270 * 10) / range));
            
            char buf[64];
            snprintf(buf, sizeof(buf), "%d km/h (%.1f°)", (int)value, angle / 10.0f);
            speedLabel->setText(buf);
            
            // Color code based on speed
            if (value < 60) {
                speedLabel->setTextColor(lv_color_hex(0x2ECC71));  // Green - safe
            } else if (value < 120) {
                speedLabel->setTextColor(lv_color_hex(0xF39C12));  // Orange - caution
            } else {
                speedLabel->setTextColor(lv_color_hex(0xE74C3C));  // Red - danger
            }
            
            ESP_LOGI(TAG, "Speed: %d km/h, Angle: %.1f°", (int)value, angle / 10.0f);
        });
        
        // Info label
        auto* infoLabel = new LVLabel(container);
        infoLabel->setText("Drag slider to change speed");
        infoLabel->setTextColor(lv_color_hex(0x95A5A6));
        infoLabel->setFont(&lv_font_montserrat_14);
        infoLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        infoLabel->setPos(0, -10);
        
        ESP_LOGI(TAG, "✅ Speedometer created");
        ESP_LOGI(TAG, "   - Range: 0-200 km/h");
        ESP_LOGI(TAG, "   - Angle: 270°, Rotation: 135°");
        ESP_LOGI(TAG, "   - Ticks: 21 total, major every 5");
    }
};

// ==================== Multi-Scale Panel ====================
class MultiScalePanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVMeter* tempScale;
    LVMeter* volumeScale;
    LVMeter* batteryMeter;
    LVSlider* tempSlider;
    LVSlider* volumeSlider;
    LVSlider* batterySlider;
    LVLabel* tempLabel;
    LVLabel* volumeLabel;
    LVLabel* batteryLabel;

public:
    MultiScalePanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 560);
        container->setPos(520, 20);
        container->setBackgroundColor(lv_color_hex(0x0F3460));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x16213E));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 20, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Multi-Scale Demo");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // === Temperature Scale (Horizontal) ===
        auto* tempTitle = new LVLabel(container);
        tempTitle->setText("Temperature");
        tempTitle->setTextColor(lv_color_hex(0xE74C3C));
        tempTitle->setFont(&lv_font_montserrat_16);
        tempTitle->setAlign(LV_ALIGN_TOP_LEFT);
        tempTitle->setPos(10, 50);
        
        tempScale = new LVMeter(container);
        tempScale->setSize(380, 50);
        tempScale->setAlign(LV_ALIGN_TOP_LEFT);
        tempScale->setPos(30, 80);
        tempScale->setMode(LV_SCALE_MODE_HORIZONTAL_BOTTOM);
        tempScale->setRange(0, 100);
        tempScale->setTotalTickCount(101);  // Every 1° for smooth section updates
        tempScale->setMajorTickEvery(10);   // Show labels at 0, 10, 20, ... 100
        
        lv_obj_set_style_bg_color(tempScale->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_length(tempScale->obj(), 10, LV_PART_INDICATOR);
        lv_obj_set_style_line_color(tempScale->obj(), lv_color_hex(0xE74C3C), LV_PART_INDICATOR);
        lv_obj_set_style_text_color(tempScale->obj(), lv_color_hex(0xECF0F1), LV_PART_INDICATOR);
        
        // Add progress section (shows filled portion from 0 to current temp)
        tempScale->addProgressSection(lv_color_hex(0xE74C3C));
        tempScale->setValue(25);  // Initial value
        
        tempSlider = new LVSlider(container);
        tempSlider->setSize(300, 15);
        tempSlider->setAlign(LV_ALIGN_TOP_LEFT);
        tempSlider->setPos(70, 140);
        tempSlider->setRange(0, 100);
        tempSlider->setValue(25);
        lv_obj_set_style_bg_color(tempSlider->obj(), lv_color_hex(0xE74C3C), LV_PART_INDICATOR);
        
        tempLabel = new LVLabel(container);
        tempLabel->setText("25°C");
        tempLabel->setTextColor(lv_color_hex(0xE74C3C));
        tempLabel->setFont(&lv_font_montserrat_14);
        tempLabel->setAlign(LV_ALIGN_TOP_LEFT);
        tempLabel->setPos(380, 135);
        
        tempSlider->onChange([this](int32_t value) {
            tempScale->setValue(value);  // Update progress section
            char buf[32];
            snprintf(buf, sizeof(buf), "%d°C (Sec)", (int)value);
            tempLabel->setText(buf);
            ESP_LOGI(TAG, "Temperature: %d°C → Section range: 0-%d", (int)value, (int)value);
        });
        
        // === Volume Scale (Vertical) ===
        auto* volumeTitle = new LVLabel(container);
        volumeTitle->setText("Volume");
        volumeTitle->setTextColor(lv_color_hex(0x3498DB));
        volumeTitle->setFont(&lv_font_montserrat_16);
        volumeTitle->setAlign(LV_ALIGN_TOP_LEFT);
        volumeTitle->setPos(10, 180);
        
        volumeScale = new LVMeter(container);
        volumeScale->setSize(50, 200);
        volumeScale->setAlign(LV_ALIGN_TOP_LEFT);
        volumeScale->setPos(40, 210);
        volumeScale->setMode(LV_SCALE_MODE_VERTICAL_LEFT);
        volumeScale->setRange(0, 10);
        volumeScale->setTotalTickCount(11);
        volumeScale->setMajorTickEvery(2);
        
        lv_obj_set_style_bg_color(volumeScale->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_length(volumeScale->obj(), 10, LV_PART_INDICATOR);
        lv_obj_set_style_line_color(volumeScale->obj(), lv_color_hex(0x3498DB), LV_PART_INDICATOR);
        lv_obj_set_style_text_color(volumeScale->obj(), lv_color_hex(0xECF0F1), LV_PART_INDICATOR);
        
        // Add progress section (shows filled portion from 0 to current volume)
        volumeScale->addProgressSection(lv_color_hex(0x3498DB));
        volumeScale->setValue(5);  // Initial value
        
        volumeSlider = new LVSlider(container);
        volumeSlider->setSize(15, 150);
        volumeSlider->setAlign(LV_ALIGN_TOP_LEFT);
        volumeSlider->setPos(100, 235);
        volumeSlider->setRange(0, 10);
        volumeSlider->setValue(7);
        lv_obj_set_style_bg_color(volumeSlider->obj(), lv_color_hex(0x3498DB), LV_PART_INDICATOR);
        
        volumeLabel = new LVLabel(container);
        volumeLabel->setText("Vol: 7");
        volumeLabel->setTextColor(lv_color_hex(0x3498DB));
        volumeLabel->setFont(&lv_font_montserrat_14);
        volumeLabel->setAlign(LV_ALIGN_TOP_LEFT);
        volumeLabel->setPos(120, 280);
        
        volumeSlider->onChange([this](int32_t value) {
            volumeScale->setValue(value);  // Update progress indicator
            char buf[16];
            snprintf(buf, sizeof(buf), "Vol: %d", (int)value);
            volumeLabel->setText(buf);
            ESP_LOGI(TAG, "Volume: %d", (int)value);
        });
        
        // === Battery Meter (Round Outer) ===
        auto* batteryTitle = new LVLabel(container);
        batteryTitle->setText("Battery");
        batteryTitle->setTextColor(lv_color_hex(0x2ECC71));
        batteryTitle->setFont(&lv_font_montserrat_16);
        batteryTitle->setAlign(LV_ALIGN_TOP_RIGHT);
        batteryTitle->setPos(-10, 180);
        
        batteryMeter = new LVMeter(container);
        batteryMeter->setSize(180, 180);
        batteryMeter->setAlign(LV_ALIGN_TOP_RIGHT);
        batteryMeter->setPos(-30, 210);
        batteryMeter->setMode(LV_SCALE_MODE_ROUND_OUTER);
        batteryMeter->setRange(0, 100);
        batteryMeter->setAngleRange(180);
        batteryMeter->setRotation(180);
        batteryMeter->setTotalTickCount(11);
        batteryMeter->setMajorTickEvery(2);
        
        lv_obj_set_style_bg_color(batteryMeter->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_length(batteryMeter->obj(), 12, LV_PART_INDICATOR);
        lv_obj_set_style_line_color(batteryMeter->obj(), lv_color_hex(0x2ECC71), LV_PART_INDICATOR);
        lv_obj_set_style_text_color(batteryMeter->obj(), lv_color_hex(0xECF0F1), LV_PART_INDICATOR);
        lv_obj_set_style_radius(batteryMeter->obj(), LV_RADIUS_CIRCLE, LV_PART_MAIN);
        
        // Add progress section (shows filled arc from 0% to current battery)
        batteryMeter->addProgressSection(lv_color_hex(0x2ECC71));
        batteryMeter->setValue(75);  // Initial value
        
        batterySlider = new LVSlider(container);
        batterySlider->setSize(120, 15);
        batterySlider->setAlign(LV_ALIGN_TOP_RIGHT);
        batterySlider->setPos(-60, 400);
        batterySlider->setRange(0, 100);
        batterySlider->setValue(75);
        lv_obj_set_style_bg_color(batterySlider->obj(), lv_color_hex(0x2ECC71), LV_PART_INDICATOR);
        
        batteryLabel = new LVLabel(container);
        batteryLabel->setText("75%");
        batteryLabel->setTextColor(lv_color_hex(0x2ECC71));
        batteryLabel->setFont(&lv_font_montserrat_18);
        batteryLabel->setAlign(LV_ALIGN_TOP_RIGHT);
        batteryLabel->setPos(-100, 300);
        
        batterySlider->onChange([this](int32_t value) {
            batteryMeter->setValue(value);  // Update progress section
            char buf[32];
            snprintf(buf, sizeof(buf), "%d%% (Sec)", (int)value);
            batteryLabel->setText(buf);
            
            // Color code battery level
            if (value > 50) {
                batteryLabel->setTextColor(lv_color_hex(0x2ECC71));  // Green
            } else if (value > 20) {
                batteryLabel->setTextColor(lv_color_hex(0xF39C12));  // Orange
            } else {
                batteryLabel->setTextColor(lv_color_hex(0xE74C3C));  // Red
            }
            
            ESP_LOGI(TAG, "Battery: %d%% → Section range: 0-%d", (int)value, (int)value);
        });
        
        // Info
        auto* infoLabel = new LVLabel(container);
        infoLabel->setText("3 scale modes: Horizontal / Vertical / Round");
        infoLabel->setTextColor(lv_color_hex(0x95A5A6));
        infoLabel->setFont(&lv_font_montserrat_14);
        infoLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        infoLabel->setPos(0, -10);
        
        ESP_LOGI(TAG, "✅ Multi-Scale Panel created");
        ESP_LOGI(TAG, "   - Temperature: Horizontal scale (0-100°C)");
        ESP_LOGI(TAG, "   - Volume: Vertical scale (0-10)");
        ESP_LOGI(TAG, "   - Battery: Round outer (0-100%)");
    }
};

// ==================== Main Test Function ====================
extern "C" void test_meter_demo() {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #16: LVMeter - Gauges");
    ESP_LOGI(TAG, "========================================");
    
    lv_obj_t* screen = lv_scr_act();
    lv_obj_clean(screen);
    
    // Set dark background
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0A0E1A), LV_PART_MAIN);
    
    // Create panels
    new SpeedometerPanel(nullptr);
    new MultiScalePanel(nullptr);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Test Cases:");
    ESP_LOGI(TAG, "✅ TC1: Round meter (Speedometer 0-200 km/h)");
    ESP_LOGI(TAG, "✅ TC2: Horizontal scale (Temperature 0-100°C)");
    ESP_LOGI(TAG, "✅ TC3: Vertical scale (Volume 0-10)");
    ESP_LOGI(TAG, "✅ TC4: Custom tick configuration");
    ESP_LOGI(TAG, "✅ TC5: Different angle ranges (270°, 180°)");
    ESP_LOGI(TAG, "✅ TC6: Multiple scale modes (ROUND/H/V)");
    ESP_LOGI(TAG, "✅ TC7: Color-coded feedback");
    ESP_LOGI(TAG, "✅ TC8: Interactive slider controls");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Instructions:");
    ESP_LOGI(TAG, "- Left: Speedometer with slider control");
    ESP_LOGI(TAG, "- Right: 3 different scale types");
    ESP_LOGI(TAG, "- Drag sliders to see meter changes");
    ESP_LOGI(TAG, "- Note: LVGL v9 uses Scale widget (not Meter)");
    ESP_LOGI(TAG, "========================================");
}
