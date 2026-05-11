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
#include <sys/stat.h>
#include <errno.h>

// System & Application managers
#include "SystemManager.h"
#include "HMINavigator.h"
#include "FileManagerApplication.h"

static const char *TAG = "main";

/**
 * @brief Main application entry point
 *
 * Architecture:
 * 1. SystemManager  — display + SD card init
 * 2. ScreenManager  — HMI navigation (Splash → Home → Library)
 */
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  LabBuddy HMI");
    ESP_LOGI(TAG, "  Splash -> Home -> Library");
    ESP_LOGI(TAG, "===========================================");

    // Step 1: Initialize display
    SystemManager &sysMgr = SystemManager::getInstance();

    ESP_LOGI(TAG, "Initializing display system...");
    if (sysMgr.initDisplay() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    // Step 2: Mount SD card (non-fatal — library may still show without it)
    ESP_LOGI(TAG, "Mounting SD card...");
    if (sysMgr.mountSDCard() != ESP_OK)
    {
        ESP_LOGW(TAG, "SD card not available — Library will show placeholder items");
    }
    else
    {
        // Ensure required directories exist so web upload can place files directly
        const char *dirs[] = {"/sdcard/lessons", "/sdcard/WORKSHOP"};
        for (const char *d : dirs)
        {
            struct stat st;
            if (stat(d, &st) != 0)
            {
                if (mkdir(d, 0775) == 0)
                    ESP_LOGI(TAG, "Created dir: %s", d);
                else
                    ESP_LOGW(TAG, "mkdir %s failed: %d", d, errno);
            }
        }
    }

    // Step 3: Start HMI — shows Splash then auto-navigates to Home
    ESP_LOGI(TAG, "Starting HMI Navigator...");
    HMINavigator::getInstance().start();

    ESP_LOGI(TAG, "HMI running");

    // Step 4: Start WiFi + Web File Manager (background task — non-fatal)
    ESP_LOGI(TAG, "Starting WiFi and Web File Manager...");
    FileManagerApplication &fileMgr = FileManagerApplication::getInstance();
    if (fileMgr.init(sysMgr) == ESP_OK)
    {
        // Pass false — HMINavigator owns the display, no FileViewerUI overlay
        if (fileMgr.start(false) == ESP_OK)
            ESP_LOGI(TAG, "Web File Manager started — IP: %s", fileMgr.getIPAddress());
        else
            ESP_LOGW(TAG, "Web File Manager failed to start (WiFi unavailable?)");
    }
    else
    {
        ESP_LOGW(TAG, "FileManagerApplication init failed");
    }

    // Keep main task alive — LVGL runs in its own task (lvgl_port)
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
