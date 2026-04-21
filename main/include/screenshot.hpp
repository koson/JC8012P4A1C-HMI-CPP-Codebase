/**
 * @file screenshot.hpp
 * @brief Screenshot utility for LVGL - Capture screen and send via UART
 */

#ifndef SCREENSHOT_HPP
#define SCREENSHOT_HPP

#include "lvgl.h"
#include "esp_log.h"
#include <stdio.h>

#if LV_USE_SNAPSHOT
#include "../../managed_components/lvgl__lvgl/src/others/snapshot/lv_snapshot.h"
#endif

class Screenshot {
public:
    /**
     * @brief Capture current screen and send via UART
     * @param obj Screen object to capture (NULL = active screen)
     * @return true if successful
     */
    static bool capture(lv_obj_t* obj = nullptr);
    
    /**
     * @brief Capture and save as BMP format via UART
     * @param obj Screen object to capture
     * @return true if successful
     */
    static bool captureBMP(lv_obj_t* obj = nullptr);

private:
    static void sendBMPHeader(uint32_t width, uint32_t height, uint32_t data_size);
    static void sendPixelData(const lv_color_t* pixels, uint32_t pixel_count);
    static const char* TAG;
};

#endif // SCREENSHOT_HPP
