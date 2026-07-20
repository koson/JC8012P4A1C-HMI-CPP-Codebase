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
#include "font_thai.h"

#define RUN_TESTS 1  // Set to 0 for normal mode

#if RUN_TESTS
#include "tests/test_all.hpp"
#endif

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
#if RUN_TESTS
    // In test mode we don't use WiFi/ESP-HOSTED, but some hosted components
    // can auto-init before app_main and retry SDIO in background.
    // Mute those tags so test output stays focused on Unity results.
    esp_log_level_set("sdmmc_common", ESP_LOG_NONE);
    esp_log_level_set("eh_host_port_sdio", ESP_LOG_NONE);
    esp_log_level_set("eh_sdio", ESP_LOG_NONE);
    esp_log_level_set("eh_reconfigure", ESP_LOG_NONE);

    run_all_tests();
    while (1) vTaskDelay(pdMS_TO_TICKS(10000));
#else



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

    // Init Thai font symbol fallback (Montserrat for LV_SYMBOL_*)
    th_niramit_init_symbols();

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
    #endif
}
