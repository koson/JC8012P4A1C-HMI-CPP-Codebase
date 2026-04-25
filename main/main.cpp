/**
 * @file main.cpp
 * @brief Entry point for FileManager Application
 *
 * This application provides:
 * - WiFi connectivity via ESP32-C6 coprocessor (ESP-HOSTED)
 * - Web-based file upload interface (HTTP server)
 * - File browser with LVGL UI
 * - SVG/JSON renderer for uploaded files
 *
 * Workflow: SVG design → JSON convert → Web upload → Select & render → Test
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// System & Application managers
#include "SystemManager.h"
#include "FileManagerApplication.h"

static const char *TAG = "main";

/**
 * @brief Main application entry point
 *
 * Architecture:
 * 1. SystemManager - handles system-level initialization (minimal for WiFi)
 * 2. FileManagerApplication - handles WiFi + HTTP server + file browsing
 */
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  LabBuddy File Manager");
    ESP_LOGI(TAG, "  WiFi + Web Upload + Renderer");
    ESP_LOGI(TAG, "===========================================");

    // Step 1: Initialize system (display, SD card required)
    SystemManager &sysMgr = SystemManager::getInstance();

    ESP_LOGI(TAG, "Initializing display system...");
    if (sysMgr.initDisplay() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    ESP_LOGI(TAG, "Mounting SD card (required for file manager)...");
    if (sysMgr.mountSDCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card - cannot continue");
        return;
    }

    // Step 2: Create and start FileManager application (Singleton)
    ESP_LOGI(TAG, "Starting FileManager application...");
    FileManagerApplication &app = FileManagerApplication::getInstance();

    if (app.init(sysMgr) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize FileManager");
        return;
    }

    ESP_LOGI(TAG, "FileManager initialized successfully");
    ESP_LOGI(TAG, "Starting WiFi and HTTP server...");

    if (app.start() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start FileManager");
        return;
    }

    // Application runs forever (WiFi + LVGL event loop)
    ESP_LOGI(TAG, "FileManager is running");

    // Keep main task alive
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
