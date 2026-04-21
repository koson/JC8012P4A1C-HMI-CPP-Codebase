/**
 * @file sd_card_test.h
 * @brief SD Card Test Functions
 */

#ifndef SD_CARD_TEST_H
#define SD_CARD_TEST_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test SD card mount
 * @return ESP_OK on success
 */
esp_err_t sd_card_test_mount(void);

/**
 * @brief Test write and read operations
 * @return ESP_OK on success
 */
esp_err_t sd_card_test_write_read(void);

/**
 * @brief List files in directory
 * @param path Directory path
 * @return ESP_OK on success
 */
esp_err_t sd_card_test_list_files(const char *path);

/**
 * @brief Test SD card unmount
 * @return ESP_OK on success
 */
esp_err_t sd_card_test_unmount(void);

/**
 * @brief Run all SD card tests
 */
void sd_card_run_all_tests(void);

#ifdef __cplusplus
}
#endif

#endif // SD_CARD_TEST_H
