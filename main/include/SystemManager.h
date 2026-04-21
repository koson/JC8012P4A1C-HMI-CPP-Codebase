#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"

/**
 * @brief SystemManager - Base system initialization manager
 * 
 * This class handles core system initialization that is common across projects:
 * - Display initialization
 * - Touch input setup
 * - SD card management
 * - System utilities
 * 
 * This is designed to be a reusable base for any ESP32-P4 HMI project.
 */
class SystemManager {
public:
    /**
     * @brief Display configuration structure
     */
    struct DisplayConfig {
        uint32_t buffer_size;           ///< Display buffer size (default: BSP_LCD_H_RES * 100)
        bool double_buffer;              ///< Enable double buffering (default: false)
        bool use_spiram;                 ///< Use SPIRAM for buffer (default: true)
        bool use_dma;                    ///< Enable DMA for buffer (default: true)
        bool sw_rotate;                  ///< Enable software rotation (default: true)
        lv_display_rotation_t rotation;  ///< Display rotation (default: LV_DISPLAY_ROTATION_270)
    };

    /**
     * @brief Get SystemManager singleton instance
     */
    static SystemManager& getInstance();

    /**
     * @brief Initialize display system
     * @param config Display configuration (nullptr = use defaults)
     * @return ESP_OK on success
     */
    esp_err_t initDisplay(const DisplayConfig* config = nullptr);

    /**
     * @brief Check touch input availability
     * @return true if touch is available
     */
    bool isTouchAvailable();

    /**
     * @brief Mount SD card
     * @return ESP_OK on success
     */
    esp_err_t mountSDCard();

    /**
     * @brief Check if SD card is mounted
     * @return true if mounted
     */
    bool isSDCardMounted() const { return m_sd_mounted; }

    /**
     * @brief Check SD card write capability
     * @param test_path Path to test write (default: "/sdcard/logs")
     * @return true if writable
     */
    bool testSDCardWrite(const char* test_path = "/sdcard/logs");

    /**
     * @brief List SD card contents
     * @param path Directory path to list (default: "/sdcard")
     */
    void listSDCardContents(const char* path = "/sdcard");

    /**
     * @brief Get LVGL display object
     * @return LVGL display object or nullptr
     */
    lv_display_t* getDisplay() const { return m_display; }

    /**
     * @brief Get LVGL touch input device
     * @return LVGL input device or nullptr
     */
    lv_indev_t* getTouchInput() const { return m_touch_indev; }

    /**
     * @brief Lock display for thread-safe LVGL operations
     * @param timeout_ms Timeout in milliseconds (-1 = wait forever)
     * @return true if locked successfully
     */
    bool lockDisplay(int timeout_ms = -1);

    /**
     * @brief Unlock display
     */
    void unlockDisplay();

    /**
     * @brief Get default display config
     */
    static DisplayConfig getDefaultDisplayConfig();

private:
    SystemManager();
    ~SystemManager() = default;
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    lv_display_t* m_display;
    lv_indev_t* m_touch_indev;
    bool m_display_initialized;
    bool m_sd_mounted;
};
