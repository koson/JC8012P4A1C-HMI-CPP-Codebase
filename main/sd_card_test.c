/**
 * @file sd_card_test.c
 * @brief SD Card Test Functions
 */

#include "sd_card_test.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "sdmmc_cmd.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static const char *TAG = "SD_TEST";

esp_err_t sd_card_test_mount(void)
{
    ESP_LOGI(TAG, "Mounting SD card...");
    esp_err_t ret = bsp_sdcard_mount();
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "SD card mounted successfully!");
    
    // Print card info
    if (bsp_sdcard != NULL) {
        sdmmc_card_print_info(stdout, bsp_sdcard);
    }
    
    return ESP_OK;
}

esp_err_t sd_card_test_write_read(void)
{
    const char *test_file = "/sdcard/test.txt";
    const char *test_data = "Hello from VPI HMI System!\nTimestamp: 2026-04-21\n";
    
    ESP_LOGI(TAG, "Testing write/read...");
    
    // Write test
    FILE *f = fopen(test_file, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    
    fprintf(f, "%s", test_data);
    fclose(f);
    ESP_LOGI(TAG, "File written successfully");
    
    // Read test
    f = fopen(test_file, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    
    char line[128];
    ESP_LOGI(TAG, "Reading file:");
    while (fgets(line, sizeof(line), f)) {
        // Strip newline
        line[strcspn(line, "\n")] = 0;
        ESP_LOGI(TAG, "  %s", line);
    }
    fclose(f);
    
    return ESP_OK;
}

esp_err_t sd_card_test_list_files(const char *path)
{
    ESP_LOGI(TAG, "Listing files in: %s", path);
    
    struct stat st;
    if (stat(path, &st) != 0) {
        ESP_LOGE(TAG, "Path does not exist: %s", path);
        return ESP_FAIL;
    }
    
    // For now, just test if we can access the mount point
    ESP_LOGI(TAG, "Mount point is accessible");
    
    return ESP_OK;
}

esp_err_t sd_card_test_unmount(void)
{
    ESP_LOGI(TAG, "Unmounting SD card...");
    esp_err_t ret = bsp_sdcard_unmount();
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unmount SD card (%s)", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "SD card unmounted successfully");
    return ESP_OK;
}

void sd_card_run_all_tests(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  SD Card Test Suite");
    ESP_LOGI(TAG, "========================================");
    
    // Test 1: Mount
    if (sd_card_test_mount() != ESP_OK) {
        ESP_LOGE(TAG, "Mount test FAILED");
        return;
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test 2: List files
    sd_card_test_list_files("/sdcard");
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test 3: Write/Read
    if (sd_card_test_write_read() != ESP_OK) {
        ESP_LOGE(TAG, "Write/Read test FAILED");
    } else {
        ESP_LOGI(TAG, "Write/Read test PASSED");
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Test 4: Unmount (optional - keep mounted for app use)
    // sd_card_test_unmount();
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  SD Card Tests Complete!");
    ESP_LOGI(TAG, "========================================");
}
