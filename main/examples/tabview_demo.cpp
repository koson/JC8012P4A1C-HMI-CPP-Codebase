/**
 * @file tabview_demo.cpp
 * @brief Demo for LVTabView - Tab container widget testing
 * 
 * Tests:
 * - Creating tabview with multiple tabs
 * - Different tab positions (top/bottom/left/right)
 * - Tab switching and callbacks
 * - Adding widgets to tab content
 */

#include "../include/LVPanel.hpp"
#include "../include/LVTabView.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVSlider.hpp"
#include "../include/LVSwitch.hpp"
#include "../include/LVTextArea.hpp"
#include "esp_log.h"

static const char* TAG = "TabViewDemo";

class TabViewDemoScreen : public LVPanel {
public:
    TabViewDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0x1E1E1E), LV_PART_MAIN);
        createUI();
    }

private:
    LVTabView* tabview = nullptr;
    LVLabel* statusLabel = nullptr;
    
    void createUI() {
        // Create tabview with tabs at top
        tabview = new LVTabView(this, LV_DIR_TOP);
        tabview->setSize(LV_PCT(95), LV_PCT(90));
        tabview->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(tabview->obj(), 10);
        
        // Tab change callback
        tabview->onTabChange([this](uint32_t tab_idx) {
            ESP_LOGI("TabViewDemo", "Tab changed to: %d", (int)tab_idx);
            if (statusLabel) {
                char buf[64];
                snprintf(buf, sizeof(buf), "Active Tab: %d", (int)tab_idx);
                statusLabel->setText(buf);
            }
        });
        
        // Create tabs
        createHomeTab();
        createSettingsTab();
        createToolsTab();
        createInfoTab();
        
        // Status label at bottom
        statusLabel = new LVLabel(this);
        statusLabel->setText("Active Tab: 0");
        statusLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(statusLabel->obj(), -10);
        statusLabel->setTextColor(lv_color_hex(0xFFFFFF));
        
        // Set initial tab
        tabview->setActiveTab(0, LV_ANIM_OFF);
        
        ESP_LOGI("TabViewDemo", "TabView demo initialized - 4 tabs created");
    }
    
    void createHomeTab() {
        lv_obj_t* tab = tabview->addTab("Home");
        
        // Title
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tab);
        title->setText("Welcome to TabView Demo!");
        title->setTextColor(lv_color_hex(0x00FF00));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 20);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_24, 0);
        
        // Description
        auto* desc = new LVLabel(nullptr);
        lv_obj_set_parent(desc->obj(), tab);
        desc->setText("This demo shows LVTabView widget\nwith multiple tabs and different content types.");
        desc->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(desc->obj(), 70);
        desc->setTextColor(lv_color_hex(0xCCCCCC));
        
        // Button
        auto* btn = new LVButton(nullptr);
        lv_obj_set_parent(btn->obj(), tab);
        btn->setSize(200, 50);
        btn->setAlign(LV_ALIGN_CENTER);
        btn->setText("Click Me!");
        
        btn->onClicked([](LVWidget* w, lv_event_t* e) {
            ESP_LOGI("TabViewDemo", "Home tab button clicked!");
        });
        
        ESP_LOGI("TabViewDemo", "Home tab created");
    }
    
    void createSettingsTab() {
        lv_obj_t* tab = tabview->addTab("Settings");
        
        // Title
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tab);
        title->setText("Settings");
        title->setTextColor(lv_color_hex(0xFF9800));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 20);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_22, 0);
        
        // Brightness slider
        auto* brightLabel = new LVLabel(nullptr);
        lv_obj_set_parent(brightLabel->obj(), tab);
        brightLabel->setText("Brightness");
        brightLabel->setPos(30, 80);
        brightLabel->setTextColor(lv_color_hex(0xFFFFFF));
        
        auto* brightSlider = new LVSlider(nullptr);
        lv_obj_set_parent(brightSlider->obj(), tab);
        brightSlider->setSize(400, 10);
        brightSlider->setPos(30, 110);
        brightSlider->setRange(0, 100);
        brightSlider->setValue(75);
        
        // Volume slider
        auto* volLabel = new LVLabel(nullptr);
        lv_obj_set_parent(volLabel->obj(), tab);
        volLabel->setText("Volume");
        volLabel->setPos(30, 150);
        volLabel->setTextColor(lv_color_hex(0xFFFFFF));
        
        auto* volSlider = new LVSlider(nullptr);
        lv_obj_set_parent(volSlider->obj(), tab);
        volSlider->setSize(400, 10);
        volSlider->setPos(30, 180);
        volSlider->setRange(0, 100);
        volSlider->setValue(50);
        
        // WiFi switch
        auto* wifiLabel = new LVLabel(nullptr);
        lv_obj_set_parent(wifiLabel->obj(), tab);
        wifiLabel->setText("WiFi");
        wifiLabel->setPos(30, 230);
        wifiLabel->setTextColor(lv_color_hex(0xFFFFFF));
        
        auto* wifiSwitch = new LVSwitch(nullptr);
        lv_obj_set_parent(wifiSwitch->obj(), tab);
        wifiSwitch->setPos(150, 225);
        wifiSwitch->setOn(true);
        
        // Bluetooth switch
        auto* btLabel = new LVLabel(nullptr);
        lv_obj_set_parent(btLabel->obj(), tab);
        btLabel->setText("Bluetooth");
        btLabel->setPos(30, 280);
        btLabel->setTextColor(lv_color_hex(0xFFFFFF));
        
        auto* btSwitch = new LVSwitch(nullptr);
        lv_obj_set_parent(btSwitch->obj(), tab);
        btSwitch->setPos(150, 275);
        btSwitch->setOn(false);
        
        ESP_LOGI("TabViewDemo", "Settings tab created");
    }
    
    void createToolsTab() {
        lv_obj_t* tab = tabview->addTab("Tools");
        
        // Title
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tab);
        title->setText("Tools & Utilities");
        title->setTextColor(lv_color_hex(0x2196F3));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 20);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_22, 0);
        
        // Text input
        auto* inputLabel = new LVLabel(nullptr);
        lv_obj_set_parent(inputLabel->obj(), tab);
        inputLabel->setText("Text Input:");
        inputLabel->setPos(30, 80);
        inputLabel->setTextColor(lv_color_hex(0xFFFFFF));
        
        auto* textarea = new LVTextArea(nullptr);
        lv_obj_set_parent(textarea->obj(), tab);
        textarea->setSize(500, 100);
        textarea->setPos(30, 110);
        textarea->setPlaceholder("Enter text here...");
        
        // Buttons
        auto* btn1 = new LVButton(nullptr);
        lv_obj_set_parent(btn1->obj(), tab);
        btn1->setSize(150, 45);
        btn1->setPos(30, 230);
        btn1->setText("Copy");
        
        auto* btn2 = new LVButton(nullptr);
        lv_obj_set_parent(btn2->obj(), tab);
        btn2->setSize(150, 45);
        btn2->setPos(200, 230);
        btn2->setText("Clear");
        
        btn2->onClicked([textarea](LVWidget* w, lv_event_t* e) {
            textarea->setText("");
            ESP_LOGI("TabViewDemo", "Text area cleared");
        });
        
        ESP_LOGI("TabViewDemo", "Tools tab created");
    }
    
    void createInfoTab() {
        lv_obj_t* tab = tabview->addTab("Info");
        
        // Title
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tab);
        title->setText("System Information");
        title->setTextColor(lv_color_hex(0x9C27B0));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 20);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_22, 0);
        
        // Info labels
        const char* infoText[] = {
            "Device: ESP32-P4",
            "Display: 1024x600 MIPI-DSI",
            "Framework: LVGL v9.2.2",
            "Language: C++23",
            "Pattern: OOP Wrapper",
            "Status: Widget Testing Phase"
        };
        
        int y_pos = 80;
        for (int i = 0; i < 6; i++) {
            auto* label = new LVLabel(nullptr);
            lv_obj_set_parent(label->obj(), tab);
            label->setText(infoText[i]);
            label->setPos(30, y_pos);
            label->setTextColor(lv_color_hex(0xCCCCCC));
            y_pos += 40;
        }
        
        // Version info
        auto* versionLabel = new LVLabel(nullptr);
        lv_obj_set_parent(versionLabel->obj(), tab);
        versionLabel->setText("Version: 1.0.0-beta");
        versionLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(versionLabel->obj(), -30);
        versionLabel->setTextColor(lv_color_hex(0x888888));
        
        ESP_LOGI("TabViewDemo", "Info tab created");
    }
};

extern "C" void create_tabview_demo() {
    auto* screen = lv_screen_active();
    auto* panel = new TabViewDemoScreen();
    lv_obj_set_parent(panel->obj(), screen);
}
