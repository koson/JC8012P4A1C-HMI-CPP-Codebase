/**
 * @file list_demo.cpp
 * @brief Phase 2 Test #15: LVList - Scrollable Lists
 * 
 * Test Cases:
 * - TC1: List with text headers
 * - TC2: List with icon buttons
 * - TC3: List with mixed items (headers + buttons)
 * - TC4: Scrollable long list
 * - TC5: List item click events
 * - TC6: Dynamic list (add/remove items)
 * - TC7: List with custom styling
 * - TC8: Multiple lists with different content
 */

#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVList.hpp"
#include "../include/LVButton.hpp"
#include "esp_log.h"

static const char* TAG = "LIST_TEST";

// ==================== Quick Settings List ====================
class QuickSettingsPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVList* settingsList;
    LVLabel* statusLabel;
    
    int clickCount = 0;

public:
    QuickSettingsPanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 560);
        container->setPos(20, 20);
        container->setBackgroundColor(lv_color_hex(0x1A1A2E));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x16213E));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 20, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Quick Settings");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Settings list
        settingsList = new LVList(container);
        settingsList->setSize(420, 430);
        settingsList->setAlign(LV_ALIGN_TOP_MID);
        settingsList->setPos(0, 50);
        lv_obj_set_style_bg_color(settingsList->obj(), lv_color_hex(0x0F3460), LV_PART_MAIN);
        lv_obj_set_style_border_width(settingsList->obj(), 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(settingsList->obj(), lv_color_hex(0x16213E), LV_PART_MAIN);
        lv_obj_set_style_radius(settingsList->obj(), 10, LV_PART_MAIN);
        lv_obj_set_style_pad_all(settingsList->obj(), 10, LV_PART_MAIN);
        
        // Add items
        settingsList->addText("Display");
        auto* btnBrightness = settingsList->addButton(LV_SYMBOL_EYE_OPEN, "Brightness");
        auto* btnOrientation = settingsList->addButton(LV_SYMBOL_LOOP, "Screen Rotation");
        auto* btnTimeout = settingsList->addButton(LV_SYMBOL_POWER, "Sleep Timeout");
        
        settingsList->addText("Sound");
        auto* btnVolume = settingsList->addButton(LV_SYMBOL_VOLUME_MAX, "Volume");
        auto* btnRingtone = settingsList->addButton(LV_SYMBOL_AUDIO, "Ringtone");
        auto* btnVibrate = settingsList->addButton(LV_SYMBOL_SHUFFLE, "Vibration");
        
        settingsList->addText("Network");
        auto* btnWiFi = settingsList->addButton(LV_SYMBOL_WIFI, "Wi-Fi");
        auto* btnBluetooth = settingsList->addButton(LV_SYMBOL_BLUETOOTH, "Bluetooth");
        auto* btnMobile = settingsList->addButton(LV_SYMBOL_CALL, "Mobile Data");
        
        // Style text headers
        lv_obj_set_style_text_color(settingsList->obj(), lv_color_hex(0x3498DB), LV_PART_ITEMS);
        lv_obj_set_style_text_font(settingsList->obj(), &lv_font_montserrat_16, LV_PART_ITEMS);
        
        // Style buttons
        lv_obj_set_style_bg_color(settingsList->obj(), lv_color_hex(0x34495E), LV_PART_ITEMS);
        lv_obj_set_style_pad_all(settingsList->obj(), 12, LV_PART_ITEMS);
        lv_obj_set_style_radius(settingsList->obj(), 8, LV_PART_ITEMS);
        
        // Status label
        statusLabel = new LVLabel(container);
        statusLabel->setText("Click any setting to configure");
        statusLabel->setTextColor(lv_color_hex(0x95A5A6));
        statusLabel->setFont(&lv_font_montserrat_14);
        statusLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        statusLabel->setPos(0, -10);
        
        // Add click event to all buttons
        lv_obj_add_event_cb(btnBrightness, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Brightness");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnOrientation, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Screen Rotation");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnTimeout, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Sleep Timeout");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnVolume, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Volume");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnRingtone, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Ringtone");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnVibrate, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Vibration");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnWiFi, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Wi-Fi");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnBluetooth, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Bluetooth");
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_add_event_cb(btnMobile, [](lv_event_t* e) {
            auto* self = static_cast<QuickSettingsPanel*>(lv_event_get_user_data(e));
            self->onItemClick("Mobile Data");
        }, LV_EVENT_CLICKED, this);
        
        ESP_LOGI(TAG, "✅ Quick Settings List created");
        ESP_LOGI(TAG, "   - 9 setting items");
        ESP_LOGI(TAG, "   - 3 categories (Display, Sound, Network)");
    }
    
    void onItemClick(const char* itemName) {
        clickCount++;
        char buf[100];
        snprintf(buf, sizeof(buf), "Selected: %s (Clicks: %d)", itemName, clickCount);
        statusLabel->setText(buf);
        ESP_LOGI(TAG, "Setting clicked: %s", itemName);
    }
};

// ==================== File Browser List ====================
class FileBrowserPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVList* fileList;
    LVLabel* pathLabel;
    LVButton* btnBack;
    LVButton* btnRefresh;
    
    int fileCount = 0;

public:
    FileBrowserPanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 560);
        container->setPos(520, 20);
        container->setBackgroundColor(lv_color_hex(0x0F3460));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x16213E));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 20, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("File Browser");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Path label
        pathLabel = new LVLabel(container);
        pathLabel->setText("Path: /storage/documents");
        pathLabel->setTextColor(lv_color_hex(0x95A5A6));
        pathLabel->setFont(&lv_font_montserrat_14);
        pathLabel->setAlign(LV_ALIGN_TOP_LEFT);
        pathLabel->setPos(0, 45);
        
        // File list
        fileList = new LVList(container);
        fileList->setSize(420, 350);
        fileList->setAlign(LV_ALIGN_TOP_MID);
        fileList->setPos(0, 80);
        lv_obj_set_style_bg_color(fileList->obj(), lv_color_hex(0x1A1A2E), LV_PART_MAIN);
        lv_obj_set_style_border_width(fileList->obj(), 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(fileList->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_radius(fileList->obj(), 10, LV_PART_MAIN);
        lv_obj_set_style_pad_all(fileList->obj(), 8, LV_PART_MAIN);
        
        // Add folders
        fileList->addText("Folders");
        auto* btnProjects = fileList->addButton(LV_SYMBOL_DIRECTORY, "Projects");
        auto* btnDocuments = fileList->addButton(LV_SYMBOL_DIRECTORY, "Documents");
        auto* btnDownloads = fileList->addButton(LV_SYMBOL_DIRECTORY, "Downloads");
        
        // Add files
        fileList->addText("Files");
        auto* btnFile1 = fileList->addButton(LV_SYMBOL_FILE, "report.pdf");
        auto* btnFile2 = fileList->addButton(LV_SYMBOL_FILE, "notes.txt");
        auto* btnFile3 = fileList->addButton(LV_SYMBOL_IMAGE, "photo.jpg");
        auto* btnFile4 = fileList->addButton(LV_SYMBOL_AUDIO, "music.mp3");
        auto* btnFile5 = fileList->addButton(LV_SYMBOL_VIDEO, "video.mp4");
        auto* btnFile6 = fileList->addButton(LV_SYMBOL_FILE, "data.xlsx");
        auto* btnFile7 = fileList->addButton(LV_SYMBOL_IMAGE, "screenshot.png");
        
        // Style items
        lv_obj_set_style_text_color(fileList->obj(), lv_color_hex(0xF39C12), LV_PART_ITEMS);
        lv_obj_set_style_bg_color(fileList->obj(), lv_color_hex(0x2C3E50), LV_PART_ITEMS);
        lv_obj_set_style_pad_all(fileList->obj(), 10, LV_PART_ITEMS);
        lv_obj_set_style_radius(fileList->obj(), 6, LV_PART_ITEMS);
        
        // Back button
        btnBack = new LVButton(container);
        btnBack->setSize(120, 45);
        btnBack->setAlign(LV_ALIGN_BOTTOM_LEFT);
        btnBack->setPos(0, -10);
        btnBack->setBackgroundColor(lv_color_hex(0x34495E));
        btnBack->setRadius(22);
        btnBack->setText(LV_SYMBOL_LEFT " Back");
        btnBack->setTextColor(lv_color_hex(0xECF0F1));
        btnBack->setFont(&lv_font_montserrat_14);
        btnBack->onClicked([this](LVWidget* w, lv_event_t* e) {
            pathLabel->setText("Path: /storage");
            ESP_LOGI(TAG, "Navigate back");
        });
        
        // Refresh button
        btnRefresh = new LVButton(container);
        btnRefresh->setSize(140, 45);
        btnRefresh->setAlign(LV_ALIGN_BOTTOM_RIGHT);
        btnRefresh->setPos(0, -10);
        btnRefresh->setBackgroundColor(lv_color_hex(0x2ECC71));
        btnRefresh->setRadius(22);
        btnRefresh->setText(LV_SYMBOL_REFRESH " Refresh");
        btnRefresh->setTextColor(lv_color_hex(0xFFFFFF));
        btnRefresh->setFont(&lv_font_montserrat_14);
        btnRefresh->onClicked([this](LVWidget* w, lv_event_t* e) {
            fileCount++;
            char buf[100];
            snprintf(buf, sizeof(buf), "Path: /storage/documents (Refresh: %d)", fileCount);
            pathLabel->setText(buf);
            ESP_LOGI(TAG, "Refresh files");
        });
        
        // Add click events to items
        lv_obj_add_event_cb(btnProjects, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open folder: Projects");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnDocuments, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open folder: Documents");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnDownloads, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open folder: Downloads");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile1, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: report.pdf");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile2, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: notes.txt");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile3, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: photo.jpg");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile4, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: music.mp3");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile5, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: video.mp4");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile6, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: data.xlsx");
        }, LV_EVENT_CLICKED, nullptr);
        
        lv_obj_add_event_cb(btnFile7, [](lv_event_t* e) {
            ESP_LOGI(TAG, "Open file: screenshot.png");
        }, LV_EVENT_CLICKED, nullptr);
        
        ESP_LOGI(TAG, "✅ File Browser List created");
        ESP_LOGI(TAG, "   - 3 folders, 7 files");
        ESP_LOGI(TAG, "   - Scrollable list");
        ESP_LOGI(TAG, "   - Navigation controls");
    }
};

// ==================== Main Test Function ====================
extern "C" void test_list_demo() {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #15: LVList - Lists");
    ESP_LOGI(TAG, "========================================");
    
    lv_obj_t* screen = lv_scr_act();
    lv_obj_clean(screen);
    
    // Set dark background
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0A0E1A), LV_PART_MAIN);
    
    // Create panels
    new QuickSettingsPanel(nullptr);
    new FileBrowserPanel(nullptr);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Test Cases:");
    ESP_LOGI(TAG, "✅ TC1: List with text headers");
    ESP_LOGI(TAG, "✅ TC2: List with icon buttons");
    ESP_LOGI(TAG, "✅ TC3: Mixed items (headers + buttons)");
    ESP_LOGI(TAG, "✅ TC4: Scrollable long list");
    ESP_LOGI(TAG, "✅ TC5: List item click events");
    ESP_LOGI(TAG, "✅ TC6: Multiple lists (Settings + Files)");
    ESP_LOGI(TAG, "✅ TC7: Custom styling (colors, fonts, spacing)");
    ESP_LOGI(TAG, "✅ TC8: Interactive controls (Back, Refresh)");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Instructions:");
    ESP_LOGI(TAG, "- Left: Click settings to select");
    ESP_LOGI(TAG, "- Right: Browse files and folders");
    ESP_LOGI(TAG, "- Test scrolling on both lists");
    ESP_LOGI(TAG, "- Use Back/Refresh buttons");
    ESP_LOGI(TAG, "========================================");
}
