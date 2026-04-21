/**
 * @file screenshot.cpp
 * @brief Screenshot implementation - Capture LVGL screen and send via UART
 */

#include "../include/screenshot.hpp"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if LV_USE_SNAPSHOT
#include "../../managed_components/lvgl__lvgl/src/others/snapshot/lv_snapshot.h"
#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_buf.h"
#endif

const char* Screenshot::TAG = "Screenshot";

void Screenshot::sendBMPHeader(uint32_t width, uint32_t height, uint32_t data_size) {
    // BMP file header (14 bytes)
    uint8_t bmp_file_header[14] = {
        'B', 'M',           // Signature
        0, 0, 0, 0,         // File size (will be filled)
        0, 0, 0, 0,         // Reserved
        54, 0, 0, 0         // Offset to pixel data
    };
    
    // BMP info header (40 bytes)
    uint8_t bmp_info_header[40] = {
        40, 0, 0, 0,        // Header size
        0, 0, 0, 0,         // Width (will be filled)
        0, 0, 0, 0,         // Height (will be filled)
        1, 0,               // Planes
        24, 0,              // Bits per pixel (24-bit RGB)
        0, 0, 0, 0,         // Compression (none)
        0, 0, 0, 0,         // Image size (will be filled)
        0, 0, 0, 0,         // X pixels per meter
        0, 0, 0, 0,         // Y pixels per meter
        0, 0, 0, 0,         // Colors used
        0, 0, 0, 0          // Important colors
    };
    
    uint32_t file_size = 54 + data_size;
    
    // Fill file size
    bmp_file_header[2] = (file_size) & 0xFF;
    bmp_file_header[3] = (file_size >> 8) & 0xFF;
    bmp_file_header[4] = (file_size >> 16) & 0xFF;
    bmp_file_header[5] = (file_size >> 24) & 0xFF;
    
    // Fill width
    bmp_info_header[4] = (width) & 0xFF;
    bmp_info_header[5] = (width >> 8) & 0xFF;
    bmp_info_header[6] = (width >> 16) & 0xFF;
    bmp_info_header[7] = (width >> 24) & 0xFF;
    
    // Fill height (negative for top-down)
    int32_t h = -(int32_t)height;
    bmp_info_header[8] = (h) & 0xFF;
    bmp_info_header[9] = (h >> 8) & 0xFF;
    bmp_info_header[10] = (h >> 16) & 0xFF;
    bmp_info_header[11] = (h >> 24) & 0xFF;
    
    // Fill image size
    bmp_info_header[20] = (data_size) & 0xFF;
    bmp_info_header[21] = (data_size >> 8) & 0xFF;
    bmp_info_header[22] = (data_size >> 16) & 0xFF;
    bmp_info_header[23] = (data_size >> 24) & 0xFF;
    
    // Send magic marker
    printf(">>>SCREENSHOT_START<<<\n");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for marker to be sent
    
    // Send headers with wait
    uart_write_bytes(UART_NUM_0, bmp_file_header, 14);
    uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(1000));
    
    uart_write_bytes(UART_NUM_0, bmp_info_header, 40);
    uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(1000));
}

void Screenshot::sendPixelData(const lv_color_t* pixels, uint32_t pixel_count) {
    // Send pixel data in smaller chunks with delays
    const uint32_t chunk_size = 256; // Reduced from 1024
    uint8_t buffer[chunk_size * 3]; // RGB888
    
    for (uint32_t i = 0; i < pixel_count; i += chunk_size) {
        uint32_t count = (i + chunk_size > pixel_count) ? (pixel_count - i) : chunk_size;
        
        // Convert to RGB888
        for (uint32_t j = 0; j < count; j++) {
            lv_color_t c = pixels[i + j];
            
            #if LV_COLOR_DEPTH == 16
            // RGB565 to RGB888
            uint8_t r = (c.red * 255) / 31;
            uint8_t g = (c.green * 255) / 63;
            uint8_t b = (c.blue * 255) / 31;
            #elif LV_COLOR_DEPTH == 32
            uint8_t r = c.red;
            uint8_t g = c.green;
            uint8_t b = c.blue;
            #else
            uint8_t r = 255;
            uint8_t g = 255;
            uint8_t b = 255;
            #endif
            
            // BMP is BGR format
            buffer[j * 3 + 0] = b;
            buffer[j * 3 + 1] = g;
            buffer[j * 3 + 2] = r;
        }
        
        // Write and wait for transmission to complete
        uart_write_bytes(UART_NUM_0, buffer, count * 3);
        uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(100));
        
        // Small delay to prevent buffer overflow
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

bool Screenshot::captureBMP(lv_obj_t* obj) {
#if LV_USE_SNAPSHOT
    if (obj == nullptr) {
        obj = lv_screen_active();
    }
    
    ESP_LOGI(TAG, "Capturing screenshot...");
    
    // Get screen dimensions
    uint32_t width = lv_obj_get_width(obj);
    uint32_t height = lv_obj_get_height(obj);
    
    ESP_LOGI(TAG, "Screen size: %dx%d", (int)width, (int)height);
    
    // Take snapshot using LVGL v9 API
    lv_draw_buf_t* snapshot = lv_snapshot_take(obj, LV_COLOR_FORMAT_RGB565);
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Failed to take snapshot");
        return false;
    }
    
    // Get pixel data from snapshot
    const lv_color_t* pixels = (const lv_color_t*)snapshot->data;
    uint32_t pixel_count = width * height;
    uint32_t data_size = pixel_count * 3; // RGB888
    
    // Send BMP header
    sendBMPHeader(width, height, data_size);
    
    // Send pixel data (row by row, bottom-up for BMP)
    ESP_LOGI(TAG, "Sending %d rows...", (int)height);
    for (int32_t y = height - 1; y >= 0; y--) {
        sendPixelData(&pixels[y * width], width);
        
        // Progress indicator every 50 rows
        if ((height - y - 1) % 50 == 0) {
            ESP_LOGI(TAG, "Progress: %d/%d rows", (int)(height - y), (int)height);
        }
    }
    
    // Cleanup
    lv_draw_buf_destroy(snapshot);
    
    // Send end marker
    printf("\n>>>SCREENSHOT_END<<<\n");
    fflush(stdout);
    
    ESP_LOGI(TAG, "Screenshot sent successfully");
    
    return true;
#else
    ESP_LOGW(TAG, "Screenshot feature disabled - LV_USE_SNAPSHOT is not enabled");
    return false;
#endif
}

bool Screenshot::capture(lv_obj_t* obj) {
    return captureBMP(obj);
}
