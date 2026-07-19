/*
 * Unit Test Main Entry Point
 * 
 * This is a standalone test runner that ONLY runs unit tests.
 * 
 * Usage:
 * 1. Backup your current main.cpp:
 *    cp main/main.cpp main/main.cpp.backup
 * 
 * 2. Copy this file to main.cpp:
 *    cp main/main_test.cpp main/main.cpp
 * 
 * 3. Update main/CMakeLists.txt to include test files:
 *    - Add tests/test_json_basic.cpp to SRCS
 *    - Add tests/test_svg_basic.cpp to SRCS
 *    - Add tests to INCLUDE_DIRS
 *    - Add unity to REQUIRES
 * 
 * 4. Build and flash:
 *    idf.py build
 *    idf.py -p COM3 flash monitor
 * 
 * 5. To restore normal operation:
 *    cp main/main.cpp.backup main/main.cpp
 *    idf.py build
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "tests/test_all.hpp"

static const char* TAG = "TEST_MAIN";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔══════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  ESP32-P4 HMI Unit Test Runner              ║");
    ESP_LOGI(TAG, "║  LabBuddy JSON & SVG Parser Tests           ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    // Wait for serial connection to stabilize
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Run all tests
    run_all_tests();
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Test run complete. System will idle.");
    ESP_LOGI(TAG, "Press RESET button to run tests again.");
    ESP_LOGI(TAG, "");
    
    // Keep system alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000));  // 1 minute
    }
}
