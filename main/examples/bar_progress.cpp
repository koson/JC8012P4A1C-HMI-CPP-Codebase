/**
 * @file bar_progress.cpp
 * @brief Phase 2 Test #10 - LVBar (Progress Indicators)
 * 
 * Tests:
 * - TC1: Create horizontal progress bar
 * - TC2: Vertical progress bar
 * - TC3: Animated value changes
 * - TC4: Range bar (start + end values)
 * - TC5: Custom colors and styling
 * - TC6: Real-time value updates
 * - TC7: Multiple bars with different modes
 */

#include "../include/LVBar.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVPanel.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string>

static const char* TAG = "BAR_TEST";

// Global widgets
static LVBar* barDownload = nullptr;
static LVBar* barUpload = nullptr;
static LVBar* barBattery = nullptr;
static LVBar* barVolume = nullptr;
static LVLabel* lblDownloadPct = nullptr;
static LVLabel* lblUploadPct = nullptr;
static LVLabel* lblBatteryPct = nullptr;
static LVLabel* lblStatus = nullptr;

// Progress values
static int32_t downloadProgress = 0;
static int32_t uploadProgress = 0;
static int32_t batteryLevel = 75;

/**
 * @brief Update download progress
 */
static void updateDownloadProgress()
{
    downloadProgress += 5;
    if (downloadProgress > 100) downloadProgress = 0;
    
    barDownload->setValue(downloadProgress, true);  // With animation
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld%%", downloadProgress);
    lblDownloadPct->setText(buffer);
    
    // Update status
    if (downloadProgress == 100) {
        lblStatus->setText("Download Complete!");
        lblStatus->setTextColor(lv_color_hex(0x388E3C));
    } else {
        char status[64];
        snprintf(status, sizeof(status), "Downloading... %ld%%", downloadProgress);
        lblStatus->setText(status);
        lblStatus->setTextColor(lv_color_hex(0x1976D2));
    }
    
    ESP_LOGI(TAG, "Download: %ld%%", downloadProgress);
}

/**
 * @brief Update upload progress
 */
static void updateUploadProgress()
{
    uploadProgress += 3;
    if (uploadProgress > 100) uploadProgress = 0;
    
    barUpload->setValue(uploadProgress, true);  // With animation
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld%%", uploadProgress);
    lblUploadPct->setText(buffer);
    
    ESP_LOGI(TAG, "Upload: %ld%%", uploadProgress);
}

/**
 * @brief Update battery level
 */
static void updateBatteryLevel(int32_t delta)
{
    batteryLevel += delta;
    if (batteryLevel > 100) batteryLevel = 100;
    if (batteryLevel < 0) batteryLevel = 0;
    
    barBattery->setValue(batteryLevel, true);  // With animation
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld%%", batteryLevel);
    lblBatteryPct->setText(buffer);
    
    // Color coding based on battery level
    if (batteryLevel < 20) {
        lv_obj_set_style_bg_color(barBattery->obj(), lv_color_hex(0xD32F2F), LV_PART_INDICATOR);
        lblBatteryPct->setTextColor(lv_color_hex(0xD32F2F));
    } else if (batteryLevel < 50) {
        lv_obj_set_style_bg_color(barBattery->obj(), lv_color_hex(0xF57C00), LV_PART_INDICATOR);
        lblBatteryPct->setTextColor(lv_color_hex(0xF57C00));
    } else {
        lv_obj_set_style_bg_color(barBattery->obj(), lv_color_hex(0x388E3C), LV_PART_INDICATOR);
        lblBatteryPct->setTextColor(lv_color_hex(0x388E3C));
    }
    
    ESP_LOGI(TAG, "Battery: %ld%%", batteryLevel);
}

/**
 * @brief Start download button callback
 */
static void onStartDownloadClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    updateDownloadProgress();
}

/**
 * @brief Start upload button callback
 */
static void onStartUploadClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    updateUploadProgress();
}

/**
 * @brief Battery charge button callback
 */
static void onChargeClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    updateBatteryLevel(10);
}

/**
 * @brief Battery drain button callback
 */
static void onDrainClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    updateBatteryLevel(-10);
}

/**
 * @brief Create Bar Progress UI
 */
extern "C" void test_bar_progress()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #10: LVBar - Progress Indicators");
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
    lblTitle->setText("Progress Bars");
    lv_obj_set_style_text_font(lblTitle->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lblTitle->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblTitle->obj(), LV_ALIGN_TOP_MID, 0, 0);
    
    // ===== Download Progress (Horizontal Bar) =====
    auto* lblDownloadTitle = new LVLabel(container);
    lblDownloadTitle->setText("Download Progress");
    lv_obj_set_style_text_font(lblDownloadTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblDownloadTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblDownloadTitle->obj(), LV_ALIGN_TOP_LEFT, 0, 50);
    
    barDownload = new LVBar(container);
    lv_obj_set_size(barDownload->obj(), 700, 30);
    lv_obj_align(barDownload->obj(), LV_ALIGN_TOP_LEFT, 0, 80);
    barDownload->setValue(0);
    
    // Styling
    lv_obj_set_style_bg_color(barDownload->obj(), lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(barDownload->obj(), lv_color_hex(0x1976D2), LV_PART_INDICATOR);
    lv_obj_set_style_radius(barDownload->obj(), 5, 0);
    
    lblDownloadPct = new LVLabel(container);
    lblDownloadPct->setText("0%");
    lv_obj_set_style_text_font(lblDownloadPct->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblDownloadPct->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblDownloadPct->obj(), LV_ALIGN_TOP_LEFT, 720, 80);
    
    auto* btnStartDownload = new LVButton(container);
    lv_obj_set_size(btnStartDownload->obj(), 140, 40);
    lv_obj_align(btnStartDownload->obj(), LV_ALIGN_TOP_LEFT, 0, 120);
    lv_obj_set_style_bg_color(btnStartDownload->obj(), lv_color_hex(0x1976D2), 0);
    btnStartDownload->setText("Start");
    btnStartDownload->setTextColor(lv_color_hex(0xFFFFFF));
    lv_obj_add_event_cb(btnStartDownload->obj(), onStartDownloadClicked, LV_EVENT_CLICKED, nullptr);
    
    // ===== Upload Progress (Horizontal Bar with different color) =====
    auto* lblUploadTitle = new LVLabel(container);
    lblUploadTitle->setText("Upload Progress");
    lv_obj_set_style_text_font(lblUploadTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblUploadTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblUploadTitle->obj(), LV_ALIGN_TOP_LEFT, 0, 180);
    
    barUpload = new LVBar(container);
    lv_obj_set_size(barUpload->obj(), 700, 30);
    lv_obj_align(barUpload->obj(), LV_ALIGN_TOP_LEFT, 0, 210);
    barUpload->setValue(0);
    
    // Styling
    lv_obj_set_style_bg_color(barUpload->obj(), lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(barUpload->obj(), lv_color_hex(0x388E3C), LV_PART_INDICATOR);
    lv_obj_set_style_radius(barUpload->obj(), 5, 0);
    
    lblUploadPct = new LVLabel(container);
    lblUploadPct->setText("0%");
    lv_obj_set_style_text_font(lblUploadPct->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblUploadPct->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblUploadPct->obj(), LV_ALIGN_TOP_LEFT, 720, 210);
    
    auto* btnStartUpload = new LVButton(container);
    lv_obj_set_size(btnStartUpload->obj(), 140, 40);
    lv_obj_align(btnStartUpload->obj(), LV_ALIGN_TOP_LEFT, 0, 250);
    lv_obj_set_style_bg_color(btnStartUpload->obj(), lv_color_hex(0x388E3C), 0);
    btnStartUpload->setText("Start");
    btnStartUpload->setTextColor(lv_color_hex(0xFFFFFF));
    lv_obj_add_event_cb(btnStartUpload->obj(), onStartUploadClicked, LV_EVENT_CLICKED, nullptr);
    
    // ===== Battery Level (Vertical Bar) =====
    auto* lblBatteryTitle = new LVLabel(container);
    lblBatteryTitle->setText("Battery");
    lv_obj_set_style_text_font(lblBatteryTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblBatteryTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblBatteryTitle->obj(), LV_ALIGN_TOP_LEFT, 0, 310);
    
    barBattery = new LVBar(container);
    lv_obj_set_size(barBattery->obj(), 40, 150);
    lv_obj_align(barBattery->obj(), LV_ALIGN_TOP_LEFT, 30, 340);
    barBattery->setValue(batteryLevel);
    
    // Styling
    lv_obj_set_style_bg_color(barBattery->obj(), lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(barBattery->obj(), lv_color_hex(0x388E3C), LV_PART_INDICATOR);
    lv_obj_set_style_radius(barBattery->obj(), 8, 0);
    
    lblBatteryPct = new LVLabel(container);
    lblBatteryPct->setText("75%");
    lv_obj_set_style_text_font(lblBatteryPct->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblBatteryPct->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblBatteryPct->obj(), LV_ALIGN_TOP_LEFT, 20, 500);
    
    // Battery control buttons
    auto* btnCharge = new LVButton(container);
    lv_obj_set_size(btnCharge->obj(), 120, 40);
    lv_obj_align(btnCharge->obj(), LV_ALIGN_TOP_LEFT, 100, 360);
    lv_obj_set_style_bg_color(btnCharge->obj(), lv_color_hex(0x388E3C), 0);
    btnCharge->setText("+10%");
    btnCharge->setTextColor(lv_color_hex(0xFFFFFF));
    lv_obj_add_event_cb(btnCharge->obj(), onChargeClicked, LV_EVENT_CLICKED, nullptr);
    
    auto* btnDrain = new LVButton(container);
    lv_obj_set_size(btnDrain->obj(), 120, 40);
    lv_obj_align(btnDrain->obj(), LV_ALIGN_TOP_LEFT, 100, 410);
    lv_obj_set_style_bg_color(btnDrain->obj(), lv_color_hex(0xD32F2F), 0);
    btnDrain->setText("-10%");
    btnDrain->setTextColor(lv_color_hex(0xFFFFFF));
    lv_obj_add_event_cb(btnDrain->obj(), onDrainClicked, LV_EVENT_CLICKED, nullptr);
    
    // ===== Volume Bar (Range Bar - shows start and end values) =====
    auto* lblVolumeTitle = new LVLabel(container);
    lblVolumeTitle->setText("Volume Range (20-80)");
    lv_obj_set_style_text_font(lblVolumeTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblVolumeTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblVolumeTitle->obj(), LV_ALIGN_TOP_LEFT, 280, 310);
    
    barVolume = new LVBar(container);
    lv_obj_set_size(barVolume->obj(), 560, 30);
    lv_obj_align(barVolume->obj(), LV_ALIGN_TOP_LEFT, 280, 340);
    barVolume->setRange(0, 100);
    barVolume->setMode(LV_BAR_MODE_RANGE);  // Range mode (shows start to end)
    barVolume->setStartValue(20, false);
    barVolume->setValue(80, false);
    
    // Styling
    lv_obj_set_style_bg_color(barVolume->obj(), lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(barVolume->obj(), lv_color_hex(0xF57C00), LV_PART_INDICATOR);
    lv_obj_set_style_radius(barVolume->obj(), 5, 0);
    
    auto* lblVolumeDesc = new LVLabel(container);
    lblVolumeDesc->setText("Range bar shows values from 20 to 80");
    lv_obj_set_style_text_font(lblVolumeDesc->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblVolumeDesc->obj(), lv_color_hex(0x757575), 0);
    lv_obj_align(lblVolumeDesc->obj(), LV_ALIGN_TOP_LEFT, 280, 380);
    
    // ===== Status Label =====
    lblStatus = new LVLabel(container);
    lblStatus->setText("Click Start buttons to test progress");
    lv_obj_set_style_text_font(lblStatus->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblStatus->obj(), lv_color_hex(0x757575), 0);
    lv_obj_align(lblStatus->obj(), LV_ALIGN_BOTTOM_MID, 0, -10);
    
    ESP_LOGI(TAG, "✅ Bar Progress UI created");
    ESP_LOGI(TAG, "   - Download bar: horizontal, animated");
    ESP_LOGI(TAG, "   - Upload bar: horizontal, different color");
    ESP_LOGI(TAG, "   - Battery bar: vertical, color-coded");
    ESP_LOGI(TAG, "   - Volume bar: range mode (20-80)");
}
