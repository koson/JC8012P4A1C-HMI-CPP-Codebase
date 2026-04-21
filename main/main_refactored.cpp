/**
 * @file main.cpp
 * @brief Entry point for ESP32-P4 HMI applications
 * 
 * This is a refactored main file that demonstrates clean separation between:
 * - System initialization (SystemManager)
 * - Application logic (VPIApplication)
 * - Entry point (app_main)
 * 
 * This architecture makes it easy to create new projects by:
 * 1. Reusing SystemManager for system setup
 * 2. Creating a new Application class for your specific app
 * 3. Keeping main.cpp minimal and clean
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// System & Application managers
#include "SystemManager.h"
#include "VPIApplication.h"

static const char* TAG = "main";

/**
 * @brief Main application entry point
 * 
 * This demonstrates clean architecture with:
 * 1. SystemManager - handles all system-level initialization
 * 2. VPIApplication - handles VPI-specific application logic
 * 3. Minimal main code - just orchestration
 */
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  ESP32-P4 HMI Application Starting");
    ESP_LOGI(TAG, "  Clean OOP Architecture");
    ESP_LOGI(TAG, "===========================================");

    // Step 1: Initialize system (display, touch, SD card)
    SystemManager& sysMgr = SystemManager::getInstance();
    
    ESP_LOGI(TAG, "Initializing display system...");
    if (sysMgr.initDisplay() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    ESP_LOGI(TAG, "Mounting SD card...");
    if (sysMgr.mountSDCard() == ESP_OK) {
        sysMgr.listSDCardContents("/sdcard");
        sysMgr.listSDCardContents("/sdcard/logs");
    } else {
        ESP_LOGW(TAG, "SD card not available - logging will be disabled");
    }

    // Step 2: Initialize application
    VPIApplication& app = VPIApplication::getInstance();
    
    ESP_LOGI(TAG, "Initializing VPI HMI Application...");
    if (app.init(sysMgr) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize application");
        return;
    }

    // Step 3: Start application
    ESP_LOGI(TAG, "Starting VPI HMI Application...");
    if (app.start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start application");
        return;
    }

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  System Ready!");
    ESP_LOGI(TAG, "===========================================");
}

// ========== C Wrapper Functions for VPI Data Manager ==========
// These allow C code to call C++ application methods

extern "C" void ui_vpi_hmi_update_electrical(uint8_t phase, float voltage, float current,
                                             float power, float reactive, float energy)
{
    VPIApplication& app = VPIApplication::getInstance();
    app.updateElectrical(phase, voltage, current, power, reactive, energy);
}

extern "C" void ui_vpi_hmi_update_pressure(float percentage)
{
    VPIApplication& app = VPIApplication::getInstance();
    app.updatePressure(percentage);
}

extern "C" void ui_vpi_hmi_update_status(const char* status)
{
    VPIApplication& app = VPIApplication::getInstance();
    app.updateStatus(status);
}
