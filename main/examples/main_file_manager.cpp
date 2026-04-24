/**
 * @file main_file_manager.cpp
 * @brief FileManager application with LVGL UI (Option 1.5)
 *
 * This demonstrates FileManagerApplication with FileViewerUI:
 * 1. Rename main.cpp to main_menu_demo.cpp
 * 2. Rename this file to main.cpp
 * 3. Build and flash
 *
 * Features:
 * - WiFi connectivity via ESP-HOSTED (ESP32-C6)
 * - Web-based file manager at http://<IP>
 * - Upload/download/delete JSON files via web
 * - LVGL UI on screen for file selection and rendering
 * - Fast iteration: Upload via web → Render on screen
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "SystemManager.h"
#include "FileManagerApplication.h"
#include "FileViewerUI.h"

static const char *TAG = "main";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  LabBuddy File Manager + Viewer");
    ESP_LOGI(TAG, "  Web Upload + Screen Render");
    ESP_LOGI(TAG, "===========================================");

    // Step 1: Initialize system (display, touch, SD card)
    SystemManager &sysMgr = SystemManager::getInstance();

    ESP_LOGI(TAG, "Initializing display system...");
    if (sysMgr.initDisplay() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    // SD card is REQUIRED for file manager
    ESP_LOGI(TAG, "Mounting SD card...");
    if (sysMgr.mountSDCard() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card - File Manager requires SD card!");
        return;
    }

    sysMgr.listSDCardContents("/sdcard");

    // Step 2: Initialize FileManager application
    FileManagerApplication &fileApp = FileManagerApplication::getInstance();

    // Optional: Custom WiFi configuration
    // FileManagerApplication::WiFiConfig wifi = {
    //     .ssid = "YOUR_SSID",
    //     .password = "YOUR_PASSWORD",
    //     .max_retry = 5,
    //     .connect_timeout_ms = 10000
    // };
    // fileApp.init(sysMgr, &wifi);

    // Use default WiFi config (aesfiber / 29052552)
    ESP_LOGI(TAG, "Initializing File Manager Application...");
    if (fileApp.init(sysMgr) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize File Manager");
        return;
    }

    // Step 3: Create FileViewerUI
    ESP_LOGI(TAG, "Creating File Viewer UI...");
    FileViewerUI viewer;
    viewer.create(lv_screen_active());

    // Step 4: Start WiFi and HTTP server
    ESP_LOGI(TAG, "Starting File Manager (WiFi + HTTP server)...");
    if (fileApp.start() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start File Manager");
        return;
    }

    // Step 5: Wait for WiFi connection and update UI
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    int retry = 0;
    while (!fileApp.isWiFiConnected() && retry < 100)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        retry++;
    }

    if (fileApp.isWiFiConnected())
    {
        ESP_LOGI(TAG, "===========================================");
        ESP_LOGI(TAG, "  File Manager Ready!");
        ESP_LOGI(TAG, "  Web UI: http://%s", fileApp.getIPAddress());
        ESP_LOGI(TAG, "===========================================");

        // Update UI with IP address
        viewer.updateIP(fileApp.getIPAddress());
    }
    else
    {
        ESP_LOGW(TAG, "WiFi not connected yet, but HTTP server is running");
        ESP_LOGW(TAG, "Will continue trying to connect in background");
        viewer.updateIP("Connecting...");
    }

    // Step 6: Main loop
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  Instructions:");
    ESP_LOGI(TAG, "  1. Open browser: http://%s", fileApp.getIPAddress());
    ESP_LOGI(TAG, "  2. Upload JSON files via web");
    ESP_LOGI(TAG, "  3. Press 'Refresh' button on screen");
    ESP_LOGI(TAG, "  4. Select file from list");
    ESP_LOGI(TAG, "  5. Press 'Render' button to view");
    ESP_LOGI(TAG, "===========================================");

    ESP_LOGI(TAG, "Application running...");

    // Keep running (LVGL handles UI events automatically)
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Optional: Update WiFi status periodically
        if (!fileApp.isWiFiConnected() && retry < 200)
        {
            retry++;
            if (fileApp.isWiFiConnected())
            {
                ESP_LOGI(TAG, "WiFi connected: %s", fileApp.getIPAddress());
                viewer.updateIP(fileApp.getIPAddress());
            }
        }
    }
}
ESP_LOGW(TAG, "WiFi not connected yet, but HTTP server is running");
ESP_LOGW(TAG, "Will continue trying to connect in background");
}

// Keep running
ESP_LOGI(TAG, "File Manager application running...");
}
