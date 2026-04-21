/**
 * @file storage_demo.cpp
 * @brief Phase 8 Test: LVStorage - File System Operations
 * 
 * Test Cases:
 * - TC1: Mount SD card
 * - TC2: Write text file
 * - TC3: Read text file
 * - TC4: Append to file
 * - TC5: List directory
 * - TC6: Get storage info
 * - TC7: Create/delete directory
 * - TC8: File operations (rename, delete)
 */

#include "../include/LVStorage.hpp"
#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "esp_log.h"
#include <sstream>

static const char* TAG = "STORAGE_TEST";

// Global UI elements
static LVLabel* g_statusLabel = nullptr;
static LVLabel* g_infoLabel = nullptr;
static LVLabel* g_contentLabel = nullptr;

// Helper to update status
static void updateStatus(const char* text, lv_color_t color = lv_color_hex(0x2ecc71)) {
    if (g_statusLabel) {
        g_statusLabel->setText(text);
        lv_obj_set_style_text_color(g_statusLabel->obj(), color, LV_PART_MAIN);
    }
}

// Test 1: Mount SD card
static void test_mount() {
    ESP_LOGI(TAG, "Test 1: Mounting SD card...");
    
    auto& storage = LVStorage::getInstance();
    
    if (storage.mount(LVStorage::SD_CARD, "/sdcard")) {
        updateStatus("✅ SD card mounted successfully", lv_color_hex(0x2ecc71));
        
        // Display storage info
        size_t total = storage.getTotalSpace();
        size_t free = storage.getFreeSpace();
        size_t used = total - free;
        
        std::ostringstream info;
        info << "Storage Info:\n";
        info << "Total: " << (total / 1024 / 1024) << " MB\n";
        info << "Used:  " << (used / 1024 / 1024) << " MB\n";
        info << "Free:  " << (free / 1024 / 1024) << " MB";
        
        if (g_infoLabel) {
            g_infoLabel->setText(info.str().c_str());
        }
        
        ESP_LOGI(TAG, "✅ Mount successful");
    } else {
        updateStatus("❌ Failed to mount SD card", lv_color_hex(0xe74c3c));
        ESP_LOGE(TAG, "❌ Mount failed");
    }
}

// Test 2: Write text file
static void test_write() {
    ESP_LOGI(TAG, "Test 2: Writing text file...");
    
    auto& storage = LVStorage::getInstance();
    
    if (!storage.isReady()) {
        updateStatus("❌ Storage not ready", lv_color_hex(0xe74c3c));
        return;
    }
    
    std::string content = "LVGL C++ Storage Test\n";
    content += "======================\n";
    content += "This is a test file created by LVStorage.\n";
    content += "Timestamp: " + std::to_string(time(nullptr)) + "\n";
    
    if (storage.writeTextFile("/sdcard/test.txt", content)) {
        updateStatus("✅ File written successfully", lv_color_hex(0x2ecc71));
        ESP_LOGI(TAG, "✅ Write successful");
    } else {
        updateStatus("❌ Failed to write file", lv_color_hex(0xe74c3c));
        ESP_LOGE(TAG, "❌ Write failed");
    }
}

// Test 3: Read text file
static void test_read() {
    ESP_LOGI(TAG, "Test 3: Reading text file...");
    
    auto& storage = LVStorage::getInstance();
    
    if (!storage.isReady()) {
        updateStatus("❌ Storage not ready", lv_color_hex(0xe74c3c));
        return;
    }
    
    std::string content;
    if (storage.readTextFile("/sdcard/test.txt", content)) {
        updateStatus("✅ File read successfully", lv_color_hex(0x2ecc71));
        
        if (g_contentLabel) {
            g_contentLabel->setText(content.c_str());
        }
        
        ESP_LOGI(TAG, "✅ Read successful");
        ESP_LOGI(TAG, "Content:\n%s", content.c_str());
    } else {
        updateStatus("❌ Failed to read file", lv_color_hex(0xe74c3c));
        ESP_LOGE(TAG, "❌ Read failed");
    }
}

// Test 4: Append to file
static void test_append() {
    ESP_LOGI(TAG, "Test 4: Appending to file...");
    
    auto& storage = LVStorage::getInstance();
    
    if (!storage.isReady()) {
        updateStatus("❌ Storage not ready", lv_color_hex(0xe74c3c));
        return;
    }
    
    std::string append_text = "--- Appended line ---\n";
    
    if (storage.appendFile("/sdcard/test.txt", 
                           (const uint8_t*)append_text.c_str(), 
                           append_text.length())) {
        updateStatus("✅ Text appended successfully", lv_color_hex(0x2ecc71));
        ESP_LOGI(TAG, "✅ Append successful");
        
        // Read and display
        test_read();
    } else {
        updateStatus("❌ Failed to append", lv_color_hex(0xe74c3c));
        ESP_LOGE(TAG, "❌ Append failed");
    }
}

// Test 5: List directory
static void test_list() {
    ESP_LOGI(TAG, "Test 5: Listing directory...");
    
    auto& storage = LVStorage::getInstance();
    
    if (!storage.isReady()) {
        updateStatus("❌ Storage not ready", lv_color_hex(0xe74c3c));
        return;
    }
    
    auto files = storage.listDir("/sdcard");
    
    updateStatus("✅ Directory listed", lv_color_hex(0x2ecc71));
    
    std::ostringstream list;
    list << "Files in /sdcard (" << files.size() << " items):\n";
    list << "========================\n";
    
    for (const auto& file : files) {
        list << "- " << file << "\n";
        ESP_LOGI(TAG, "  - %s", file.c_str());
    }
    
    if (g_contentLabel) {
        g_contentLabel->setText(list.str().c_str());
    }
}

// Test 6: Create directory
static void test_mkdir() {
    ESP_LOGI(TAG, "Test 6: Creating directory...");
    
    auto& storage = LVStorage::getInstance();
    
    if (!storage.isReady()) {
        updateStatus("❌ Storage not ready", lv_color_hex(0xe74c3c));
        return;
    }
    
    if (storage.createDir("/sdcard/testdir")) {
        updateStatus("✅ Directory created", lv_color_hex(0x2ecc71));
        ESP_LOGI(TAG, "✅ Directory created");
        
        // Create a file inside
        storage.writeTextFile("/sdcard/testdir/info.txt", "Test directory file\n");
        
        // List to verify
        test_list();
    } else {
        updateStatus("⚠️ Directory exists or failed", lv_color_hex(0xf39c12));
    }
}

// Test 7: Delete file
static void test_delete() {
    ESP_LOGI(TAG, "Test 7: Deleting file...");
    
    auto& storage = LVStorage::getInstance();
    
    if (!storage.isReady()) {
        updateStatus("❌ Storage not ready", lv_color_hex(0xe74c3c));
        return;
    }
    
    if (storage.deleteFile("/sdcard/test.txt")) {
        updateStatus("✅ File deleted", lv_color_hex(0x2ecc71));
        ESP_LOGI(TAG, "✅ Delete successful");
        
        test_list();
    } else {
        updateStatus("❌ Failed to delete file", lv_color_hex(0xe74c3c));
    }
}

// Main test function
extern "C" void test_storage_demo() {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 8 Test: LVStorage - File System");
    ESP_LOGI(TAG, "========================================");
    
    lv_obj_t* screen = lv_scr_act();
    lv_obj_clean(screen);
    
    // Dark background
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0A0E1A), LV_PART_MAIN);
    
    // Title
    LVLabel* title = new LVLabel(nullptr);
    title->setText("🗂️ LVStorage - File System Demo");
    lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(title->obj(), lv_color_hex(0xffffff), LV_PART_MAIN);
    title->setAlign(LV_ALIGN_TOP_MID);
    title->setPos(0, 10);
    
    // Status panel
    LVPanel* statusPanel = new LVPanel(nullptr);
    statusPanel->setSize(1000, 80);
    lv_obj_set_style_bg_color(statusPanel->obj(), lv_color_hex(0x16213e), LV_PART_MAIN);
    statusPanel->setPos(12, 50);
    
    g_statusLabel = new LVLabel(statusPanel);
    g_statusLabel->setText("⏳ Ready - Click button to start");
    lv_obj_set_style_text_font(g_statusLabel->obj(), &lv_font_montserrat_18, LV_PART_MAIN);
    g_statusLabel->setAlign(LV_ALIGN_CENTER);
    
    // Info panel (storage info)
    LVPanel* infoPanel = new LVPanel(nullptr);
    infoPanel->setSize(480, 120);
    lv_obj_set_style_bg_color(infoPanel->obj(), lv_color_hex(0x1a2332), LV_PART_MAIN);
    infoPanel->setPos(12, 140);
    
    g_infoLabel = new LVLabel(infoPanel);
    g_infoLabel->setText("Storage info will appear here");
    lv_obj_set_style_text_color(g_infoLabel->obj(), lv_color_hex(0xecf0f1), LV_PART_MAIN);
    g_infoLabel->setAlign(LV_ALIGN_TOP_LEFT);
    g_infoLabel->setPos(10, 10);
    
    // Content panel (file content / directory listing)
    LVPanel* contentPanel = new LVPanel(nullptr);
    contentPanel->setSize(480, 290);
    lv_obj_set_style_bg_color(contentPanel->obj(), lv_color_hex(0x1a2332), LV_PART_MAIN);
    contentPanel->setPos(12, 270);
    
    g_contentLabel = new LVLabel(contentPanel);
    g_contentLabel->setText("File content will appear here");
    lv_obj_set_style_text_color(g_contentLabel->obj(), lv_color_hex(0x95a5a6), LV_PART_MAIN);
    lv_obj_set_style_text_font(g_contentLabel->obj(), &lv_font_montserrat_14, LV_PART_MAIN);
    g_contentLabel->setAlign(LV_ALIGN_TOP_LEFT);
    g_contentLabel->setPos(10, 10);
    
    // Button panel
    LVPanel* btnPanel = new LVPanel(nullptr);
    btnPanel->setSize(510, 420);
    lv_obj_set_style_bg_color(btnPanel->obj(), lv_color_hex(0x16213e), LV_PART_MAIN);
    btnPanel->setPos(502, 140);
    
    LVLabel* btnTitle = new LVLabel(btnPanel);
    btnTitle->setText("Test Operations");
    lv_obj_set_style_text_font(btnTitle->obj(), &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(btnTitle->obj(), lv_color_hex(0x3498db), LV_PART_MAIN);
    btnTitle->setAlign(LV_ALIGN_TOP_MID);
    btnTitle->setPos(0, 10);
    
    // Create test buttons
    const char* button_labels[] = {
        "1. Mount SD Card",
        "2. Write File",
        "3. Read File",
        "4. Append Text",
        "5. List Directory",
        "6. Create Dir",
        "7. Delete File"
    };
    
    void (*button_funcs[])() = {
        test_mount,
        test_write,
        test_read,
        test_append,
        test_list,
        test_mkdir,
        test_delete
    };
    
    for (int i = 0; i < 7; i++) {
        LVButton* btn = new LVButton(btnPanel);
        btn->setText(button_labels[i]);
        btn->setSize(470, 45);
        lv_obj_set_style_bg_color(btn->obj(), lv_color_hex(0x2c3e50), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn->obj(), lv_color_hex(0x3498db), LV_STATE_PRESSED);
        btn->setPos(20, 50 + i * 52);
        
        void (*func)() = button_funcs[i];
        lv_obj_add_event_cb(btn->obj(), [](lv_event_t* e) {
            void (*callback)() = (void(*)())lv_event_get_user_data(e);
            if (callback) callback();
        }, LV_EVENT_CLICKED, (void*)func);
    }
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Instructions:");
    ESP_LOGI(TAG, "1. Click 'Mount SD Card' first");
    ESP_LOGI(TAG, "2. Test write/read operations");
    ESP_LOGI(TAG, "3. View storage info and file content");
    ESP_LOGI(TAG, "========================================");
}

extern "C" void cleanup_storage_demo() {
    ESP_LOGI(TAG, "Cleaning up storage demo...");
    
    // Note: Don't unmount here - it causes issues on next mount
    // Let the SD card stay mounted for reuse by other demos
    
    g_statusLabel = nullptr;
    g_infoLabel = nullptr;
    g_contentLabel = nullptr;
    
    ESP_LOGI(TAG, "Storage demo cleanup complete");
}
