/**
 * @file speedometer_with_needle.cpp
 * @brief Speedometer with Image Needle - Using LVGL Demo Needle
 * 
 * Features:
 * - Real needle image from LVGL demo
 * - Smooth rotation animation
 * - Color-coded speed zones
 * - Interactive speed control
 */

#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVSlider.hpp"
#include "../include/LVButton.hpp"
#include "img_demo_widgets_needle.h"
#include "esp_log.h"
#include "lvgl.h"

static const char* TAG = "SPEEDOMETER_NEEDLE";

// Needle image is already declared in the header file

class SpeedometerNeedleDemo {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    lv_obj_t* scale;        // Scale object (C API for scale)
    lv_obj_t* needle;       // Needle image
    LVSlider* speedSlider;
    LVLabel* speedLabel;
    LVLabel* infoLabel;
    
    int32_t currentSpeed = 0;

public:
    SpeedometerNeedleDemo(LVWidget* parent) {
        // Main Container
        container = new LVPanel(parent);
        container->setSize(480, 600);
        container->setPos(20, 20);
        container->setBackgroundColor(lv_color_hex(0x0F0F1E));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x3498DB));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 20, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Speedometer with Needle");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        createSpeedometer();
        createControls();
        
        ESP_LOGI(TAG, "✅ Speedometer with needle created");
    }
    
private:
    void createSpeedometer() {
        // Create scale using C API (for better needle control)
        scale = lv_scale_create(container->obj());
        lv_obj_set_size(scale, 350, 350);
        lv_obj_align(scale, LV_ALIGN_TOP_MID, 0, 60);
        
        // Scale configuration
        lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
        lv_scale_set_range(scale, 0, 200);           // 0-200 km/h
        lv_scale_set_angle_range(scale, 270);        // 270° span
        lv_scale_set_rotation(scale, 135);           // Start from top-left
        lv_scale_set_total_tick_count(scale, 21);    // 0, 10, 20, ..., 200
        lv_scale_set_major_tick_every(scale, 5);     // Major tick every 50 km/h
        
        // Style the scale background
        lv_obj_set_style_bg_color(scale, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
        lv_obj_set_style_border_width(scale, 4, LV_PART_MAIN);
        lv_obj_set_style_border_color(scale, lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_radius(scale, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(scale, LV_OPA_90, LV_PART_MAIN);
        
        // Major tick styling (every 50 km/h)
        lv_obj_set_style_length(scale, 25, LV_PART_INDICATOR);
        lv_obj_set_style_line_width(scale, 4, LV_PART_INDICATOR);
        lv_obj_set_style_line_color(scale, lv_color_hex(0xE74C3C), LV_PART_INDICATOR);
        
        // Minor tick styling
        lv_obj_set_style_length(scale, 15, LV_PART_ITEMS);
        lv_obj_set_style_line_width(scale, 2, LV_PART_ITEMS);
        lv_obj_set_style_line_color(scale, lv_color_hex(0xBDC3C7), LV_PART_ITEMS);
        
        // Tick label styling
        lv_obj_set_style_text_color(scale, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
        lv_obj_set_style_text_font(scale, &lv_font_montserrat_16, LV_PART_INDICATOR);
        
        // Add colored sections
        addSpeedSections();
        
        // Create needle image
        createNeedle();
        
        // Center dot
        lv_obj_t* center = lv_obj_create(scale);
        lv_obj_set_size(center, 16, 16);
        lv_obj_align(center, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(center, lv_color_hex(0xE74C3C), LV_PART_MAIN);
        lv_obj_set_style_border_color(center, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(center, 2, LV_PART_MAIN);
        lv_obj_set_style_radius(center, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    }
    
    void addSpeedSections() {
        // Green zone (0-60 km/h) - Safe
        lv_scale_section_t* section1 = lv_scale_add_section(scale);
        lv_scale_set_section_range(scale, section1, 0, 60);
        
        static lv_style_t green_style;
        lv_style_init(&green_style);
        lv_style_set_arc_color(&green_style, lv_color_hex(0x2ECC71));
        lv_style_set_arc_width(&green_style, 8);
        lv_scale_set_section_style_main(scale, section1, &green_style);
        
        // Yellow zone (60-120 km/h) - Caution  
        lv_scale_section_t* section2 = lv_scale_add_section(scale);
        lv_scale_set_section_range(scale, section2, 60, 120);
        
        static lv_style_t yellow_style;
        lv_style_init(&yellow_style);
        lv_style_set_arc_color(&yellow_style, lv_color_hex(0xF39C12));
        lv_style_set_arc_width(&yellow_style, 8);
        lv_scale_set_section_style_main(scale, section2, &yellow_style);
        
        // Red zone (120-200 km/h) - Danger
        lv_scale_section_t* section3 = lv_scale_add_section(scale);
        lv_scale_set_section_range(scale, section3, 120, 200);
        
        static lv_style_t red_style;
        lv_style_init(&red_style);
        lv_style_set_arc_color(&red_style, lv_color_hex(0xE74C3C));
        lv_style_set_arc_width(&red_style, 8);
        lv_scale_set_section_style_main(scale, section3, &red_style);
    }
    
    void createNeedle() {
        // Create needle using the LVGL demo image
        needle = lv_image_create(scale);
        lv_image_set_src(needle, &img_demo_widgets_needle);
        
        // Set pivot point (near the base of needle)
        lv_image_set_pivot(needle, 3, 4);
        
        // Position needle at center
        lv_obj_align(needle, LV_ALIGN_CENTER, 45, 0);
        
        // Initial position (0 km/h)
        lv_scale_set_image_needle_value(scale, needle, 0);
        
        ESP_LOGI(TAG, "Needle created and positioned");
    }
    
    void createControls() {
        // Speed display label
        speedLabel = new LVLabel(container);
        speedLabel->setText("0 km/h");
        speedLabel->setTextColor(lv_color_hex(0x2ECC71));
        speedLabel->setFont(&lv_font_montserrat_32);
        speedLabel->setAlign(LV_ALIGN_CENTER);
        speedLabel->setPos(0, 100);
        
        // Speed control slider
        speedSlider = new LVSlider(container);
        speedSlider->setSize(400, 25);
        speedSlider->setAlign(LV_ALIGN_BOTTOM_MID);
        speedSlider->setPos(0, -80);
        speedSlider->setRange(0, 200);
        speedSlider->setValue(0);
        
        // Slider styling
        lv_obj_set_style_bg_color(speedSlider->obj(), lv_color_hex(0x3498DB), LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(speedSlider->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_radius(speedSlider->obj(), 12, LV_PART_MAIN | LV_PART_INDICATOR);
        
        // Slider event handler
        speedSlider->onChange([this](int32_t value) {
            updateSpeed(value);
        });
        
        // Control buttons
        createSpeedButtons();
        
        // Info label
        infoLabel = new LVLabel(container);
        infoLabel->setText("Use slider or buttons to control speed");
        infoLabel->setTextColor(lv_color_hex(0x95A5A6));
        infoLabel->setFont(&lv_font_montserrat_14);
        infoLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        infoLabel->setPos(0, -10);
    }
    
    void createSpeedButtons() {
        // Speed preset buttons
        int speeds[] = {0, 30, 60, 90, 120, 160, 200};
        const char* labels[] = {"0", "30", "60", "90", "120", "160", "MAX"};
        lv_color_t colors[] = {
            lv_color_hex(0x95A5A6),   // 0 - Gray
            lv_color_hex(0x2ECC71),   // 30 - Green  
            lv_color_hex(0x2ECC71),   // 60 - Green
            lv_color_hex(0xF39C12),   // 90 - Orange
            lv_color_hex(0xF39C12),   // 120 - Orange
            lv_color_hex(0xE74C3C),   // 160 - Red
            lv_color_hex(0x8E44AD)    // MAX - Purple
        };
        
        for(int i = 0; i < 7; i++) {
            auto* btn = new LVButton(container);
            btn->setSize(50, 35);
            btn->setPos(-150 + (i * 50), -45);
            
            // Button styling
            lv_obj_set_style_bg_color(btn->obj(), colors[i], LV_PART_MAIN);
            lv_obj_set_style_radius(btn->obj(), 8, LV_PART_MAIN);
            
            auto* btnLabel = new LVLabel(btn);
            btnLabel->setText(labels[i]);
            btnLabel->setTextColor(lv_color_hex(0xFFFFFF));
            btnLabel->setFont(&lv_font_montserrat_12);
            btnLabel->setAlign(LV_ALIGN_CENTER);
            
            // Capture speed value for lambda
            int speed = speeds[i];
            btn->onClicked([this, speed](LVWidget* widget, lv_event_t* e) {
                speedSlider->setValue(speed);
                updateSpeed(speed);
            });
        }
    }
    
    void updateSpeed(int32_t speed) {
        currentSpeed = speed;
        
        // Update needle position with smooth animation
        lv_scale_set_image_needle_value(scale, needle, speed);
        
        // Update speed label with color coding
        char buf[32];
        snprintf(buf, sizeof(buf), "%d km/h", (int)speed);
        speedLabel->setText(buf);
        
        // Color coding based on speed zones
        lv_color_t color;
        const char* zone;
        if (speed <= 60) {
            color = lv_color_hex(0x2ECC71);  // Green - Safe
            zone = "SAFE";
        } else if (speed <= 120) {
            color = lv_color_hex(0xF39C12);  // Orange - Caution
            zone = "CAUTION";
        } else {
            color = lv_color_hex(0xE74C3C);  // Red - Danger
            zone = "DANGER";
        }
        
        speedLabel->setTextColor(color);
        
        // Update info with zone information
        snprintf(buf, sizeof(buf), "Speed: %d km/h - Zone: %s", (int)speed, zone);
        infoLabel->setText(buf);
        
        ESP_LOGI(TAG, "Speed updated: %d km/h (%s zone)", (int)speed, zone);
    }
};

// Simple test function that can be called from main.cpp
extern "C" void run_speedometer_needle_test() {
    ESP_LOGI(TAG, "🚀 Starting Speedometer Needle Test...");
    
    // Create speedometer demo directly on screen
    new SpeedometerNeedleDemo(nullptr);  // Use nullptr for screen root
    
    ESP_LOGI(TAG, "✅ Speedometer needle test initialized");
}