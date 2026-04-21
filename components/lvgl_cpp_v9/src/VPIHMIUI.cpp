/**
 * @file VPIHMIUI.cpp
 * @brief VPI HMI Main UI Implementation
 */

#include "VPIHMIUI.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include <cstdio>

static const char *TAG = "VPIHMIUI";

// Screen dimensions - Landscape 1280×800
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 800

// External needle image
extern "C" {
    LV_IMAGE_DECLARE(img_pressure_needle);
}

VPIHMIUI::VPIHMIUI()
    : screen_(nullptr)
    , header_panel_(nullptr)
    , title_label_(nullptr)
    , log_button_(nullptr)
    , datetime_label_(nullptr)
    , electrical_panel_(nullptr)
    , tank_panel_(nullptr)
    , tank_gauge_(nullptr)
    , status_label_(nullptr)
{
    ESP_LOGI(TAG, "Creating VPI HMI UI (C++ OOP, Landscape 1280x800)...");
    
    // Get active screen
    screen_ = lv_scr_act();
    
    // Set black background
    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, 0);
    
    // Create header bar (top)
    createHeaderBar();
    
    // Create left panel: Electrical parameters
    electrical_panel_ = new ElectricalPanel(screen_);
    
    // Create right panel: Tank gauge
    createTankPanel();
    tank_gauge_ = new VPITankGauge(tank_panel_, &img_pressure_needle);
    tank_gauge_->setPosition(50, 100);  // Center in panel
    
    // Create status bar
    createStatusBar();
    
    ESP_LOGI(TAG, "VPI HMI UI created successfully (C++ OOP)");
}

VPIHMIUI::~VPIHMIUI()
{
    delete tank_gauge_;
    delete electrical_panel_;
}

void VPIHMIUI::updateElectrical(uint8_t phase, float voltage, float current,
                                float power, float reactive, float energy)
{
    if (electrical_panel_) {
        electrical_panel_->updatePhase(phase, voltage, current, power, reactive, energy);
    }
}

void VPIHMIUI::updatePressure(float percentage)
{
    if (tank_gauge_) {
        // Lock display for thread-safe update
        if (bsp_display_lock(100)) {
            tank_gauge_->updatePressure(percentage);
            bsp_display_unlock();
        }
    }
}

void VPIHMIUI::updateStatus(const char* status)
{
    if (status_label_) {
        // Lock display for thread-safe update
        if (bsp_display_lock(100)) {
            lv_label_set_text(status_label_, status);
            bsp_display_unlock();
        }
    }
}

void VPIHMIUI::updateDateTime(const char* datetime)
{
    if (datetime_label_) {
        // Lock display for thread-safe update
        if (bsp_display_lock(100)) {
            lv_label_set_text(datetime_label_, datetime);
            bsp_display_unlock();
        }
    }
}

void VPIHMIUI::setLoggingState(bool is_logging)
{
    ESP_LOGI("VPIHMIUI", "setLoggingState() called: is_logging=%d, log_button_=%p", 
             is_logging, log_button_);
    
    if (log_button_) {
        ESP_LOGI("VPIHMIUI", "Attempting to lock display...");
        if (bsp_display_lock(100)) {
            ESP_LOGI("VPIHMIUI", "Display locked, updating button style...");
            
            if (is_logging) {
                lv_obj_add_state(log_button_, LV_STATE_CHECKED);
                lv_obj_set_style_bg_color(log_button_, lv_color_hex(0xFF0000), LV_STATE_CHECKED);
                lv_label_set_text(lv_obj_get_child(log_button_, 0), "STOP");  // กำลัง log อยู่ → กด = หยุด
                ESP_LOGI("VPIHMIUI", "✓ Button set to RED (CHECKED) - STOP");
            } else {
                lv_obj_remove_state(log_button_, LV_STATE_CHECKED);
                lv_obj_set_style_bg_color(log_button_, lv_color_hex(0x00AA00), 0);
                lv_label_set_text(lv_obj_get_child(log_button_, 0), "START");  // ไม่ได้ log → กด = เริ่ม
                ESP_LOGI("VPIHMIUI", "✓ Button set to GREEN (UNCHECKED) - START");
            }
            bsp_display_unlock();
        } else {
            ESP_LOGE("VPIHMIUI", "✗ Failed to lock display (timeout)!");
        }
    } else {
        ESP_LOGE("VPIHMIUI", "✗ log_button_ is NULL!");
    }
}

void VPIHMIUI::createHeaderBar()
{
    // Header panel (full width, 60px height)
    header_panel_ = lv_obj_create(screen_);
    lv_obj_set_size(header_panel_, SCREEN_WIDTH, 60);
    lv_obj_set_pos(header_panel_, 0, 0);
    lv_obj_set_style_bg_color(header_panel_, lv_color_hex(0x0a0a0a), 0);
    lv_obj_set_style_bg_opa(header_panel_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header_panel_, 0, 0);
    lv_obj_set_style_border_side(header_panel_, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(header_panel_, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(header_panel_, 2, 0);
    lv_obj_set_style_pad_all(header_panel_, 10, 0);
    lv_obj_clear_flag(header_panel_, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title label (left): "VPI SYSTEM"
    title_label_ = lv_label_create(header_panel_);
    lv_label_set_text(title_label_, "VPI SYSTEM");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_28, 0);
    lv_obj_align(title_label_, LV_ALIGN_LEFT_MID, 10, 0);
    
    // LOG button (center)
    log_button_ = lv_button_create(header_panel_);
    lv_obj_set_size(log_button_, 180, 45);
    lv_obj_align(log_button_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(log_button_, lv_color_hex(0x00AA00), 0);  // Green = START
    lv_obj_set_style_bg_color(log_button_, lv_color_hex(0xFF0000), LV_STATE_CHECKED);  // Red = STOP
    lv_obj_set_style_radius(log_button_, 5, 0);
    
    // Enable checkable/toggle behavior
    lv_obj_add_flag(log_button_, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_flag(log_button_, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_t* btn_label = lv_label_create(log_button_);
    lv_label_set_text(btn_label, "LOG START");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_20, 0);
    lv_obj_center(btn_label);
    
    // DateTime label (right)
    datetime_label_ = lv_label_create(header_panel_);
    lv_label_set_text(datetime_label_, "2026-04-21 00:00:00");
    lv_obj_set_style_text_color(datetime_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(datetime_label_, &lv_font_montserrat_20, 0);
    lv_obj_align(datetime_label_, LV_ALIGN_RIGHT_MID, -10, 0);
}

void VPIHMIUI::createTankPanel()
{
    tank_panel_ = lv_obj_create(screen_);
    lv_obj_set_size(tank_panel_, 440, 680);  // Height reduced to 680 to leave space for status bar
    lv_obj_set_pos(tank_panel_, SCREEN_WIDTH - 450, 70);  // Y=70 to leave space for header
    lv_obj_set_style_bg_color(tank_panel_, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_bg_opa(tank_panel_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tank_panel_, 2, 0);
    lv_obj_set_style_border_color(tank_panel_, lv_color_hex(0x00FF00), 0);  // Green border
    lv_obj_set_style_pad_all(tank_panel_, 10, 0);
    lv_obj_clear_flag(tank_panel_, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title
    lv_obj_t* title = lv_label_create(tank_panel_);
    lv_label_set_text(title, "Tank parameters");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(title, 10, 5);
}

void VPIHMIUI::createStatusBar()
{
    status_label_ = lv_label_create(screen_);
    lv_label_set_text(status_label_, "Status: Waiting for data...");
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(status_label_, &lv_font_montserrat_14, 0);
    lv_obj_align(status_label_, LV_ALIGN_BOTTOM_LEFT, 10, -5);
}
