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

#define RUN_TESTS 0  // Set to 1 for unit-test mode

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
    ESP_LOGI(TAG, "  LabBuddy HMI 260720");
    ESP_LOGI(TAG, "  Splash -> Home -> Library");
    ESP_LOGI(TAG, "===========================================");

//   while (1)  {vTaskDelay(pdMS_TO_TICKS(10000));}
  

    // Step 1: Initialize system services needed for the web server.
    // WiFi must come up before SD card mounting when ESP-Hosted uses SDIO.
    SystemManager &sysMgr = SystemManager::getInstance();

    // Step 2: Start WiFi + Web File Manager (background task — non-fatal)
    ESP_LOGI(TAG, "Starting WiFi and Web File Manager...");
    FileManagerApplication &fileMgr = FileManagerApplication::getInstance();
    bool fileMgr_started = false;
    int wifi_retry_tick = 0;
    if (fileMgr.init(sysMgr) == ESP_OK)
    {
        if (fileMgr.start(false) == ESP_OK)
        {
            ESP_LOGI(TAG, "Web File Manager started — IP: %s", fileMgr.getIPAddress());
            fileMgr_started = true;
        }
        else
            ESP_LOGW(TAG, "Web File Manager failed to start (WiFi unavailable?)");
    }
    else
    {
        ESP_LOGW(TAG, "FileManagerApplication init failed");
    }

    // Step 3: Mount SD card after ESP-Hosted WiFi is already active.
    // This matches the hosted SDIO + SD card combined workaround.
    ESP_LOGI(TAG, "Mounting SD card...");
    if (sysMgr.mountSDCard() != ESP_OK)
    {
        ESP_LOGW(TAG, "SD card not available — File uploads and lesson fetch will be limited");
    }
    else
    {
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

    // Keep main task alive — LVGL runs in its own task (lvgl_port)
    while (true)
    {
        if (!fileMgr_started)
        {
            wifi_retry_tick += 10;
            if (wifi_retry_tick >= 15)
            {
                ESP_LOGI(TAG, "Retrying WiFi/Web File Manager startup...");
                if (fileMgr.start(false) == ESP_OK)
                {
                    fileMgr_started = true;
                    ESP_LOGI(TAG, "Web File Manager recovery succeeded — IP: %s", fileMgr.getIPAddress());
                }
                wifi_retry_tick = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    #endif
}
