/**
 * @file main.cpp
 * @brief Entry point for ESP32-P4 HMI applications
 * 
 * This is a refactored main file that demonstrates clean separation between:
 * - System initialization (SystemManager)
 * - Application logic (MenuDemoApplication)
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
#include "MenuDemoApplication.h"

static const char* TAG = "main";

/**
 * @brief Main application entry point
 * 
 * This demonstrates clean architecture with:
 * 1. SystemManager - handles all system-level initialization
 * 2. MenuDemoApplication - handles menu demo gallery
 * 3. Minimal main code - just orchestration
 */
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  ESP32-P4 Demo Gallery");
    ESP_LOGI(TAG, "  Clean OOP Architecture");
    ESP_LOGI(TAG, "===========================================");

    // Step 1: Initialize system (display, touch, SD card)
    SystemManager& sysMgr = SystemManager::getInstance();
    
    ESP_LOGI(TAG, "Initializing display system...");
    if (sysMgr.initDisplay() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    // SD card is optional for menu demo
    ESP_LOGI(TAG, "Mounting SD card (optional)...");
    if (sysMgr.mountSDCard() == ESP_OK) {
        sysMgr.listSDCardContents("/sdcard");
    } else {
        ESP_LOGW(TAG, "SD card not available - some demos may not work");
    }

    // Step 2: Initialize application
    MenuDemoApplication& app = MenuDemoApplication::getInstance();
    
    ESP_LOGI(TAG, "Initializing Menu Demo Application...");
    if (app.init(sysMgr) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize application");
        return;
    }

    // Step 3: Start application
    ESP_LOGI(TAG, "Starting Menu Demo Application...");
    if (app.start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start application");
        return;
    }

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  Demo Gallery Ready!");
    ESP_LOGI(TAG, "===========================================");
}

// No C wrapper functions needed for MenuDemoApplication
// (Menu demo doesn't interface with C components)
