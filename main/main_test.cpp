/*
 * Simple Unit Test Runner
 * 
 * Usage: Flash this to ESP32-P4, open serial monitor to see test results
 * 
 * This file integrates unit tests directly into main.cpp for easy testing
 */

#include "unity.h"
#include "unity_test_runner.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Include test suites
#include "tests/test_json_parser.hpp"
#include "tests/test_svg_path_parser.hpp"

static const char* TAG = "TEST_RUNNER";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "  ESP32-P4 HMI Unit Test Runner");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");
    
    // Wait a bit for serial connection
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Initialize Unity test framework
    UNITY_BEGIN();
    
    // Run JSON Parser tests
    ESP_LOGI(TAG, "Running JSON Parser Tests...");
    run_json_parser_tests();
    
    // Run SVG Path Parser tests
    ESP_LOGI(TAG, "Running SVG Path Parser Tests...");
    run_svg_path_parser_tests();
    
    // Finish testing
    int failures = UNITY_END();
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    if (failures == 0) {
        ESP_LOGI(TAG, "✅ ALL TESTS PASSED!");
    } else {
        ESP_LOGE(TAG, "❌ %d TEST(S) FAILED!", failures);
    }
    ESP_LOGI(TAG, "==============================================");
    
    // Keep running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
