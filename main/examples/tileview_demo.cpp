/**
 * @file tileview_demo.cpp
 * @brief Demo for LVTileView - Swipeable pages widget testing
 * 
 * Tests:
 * - TileView with multiple tiles
 * - Swipe navigation between tiles
 * - Tile direction control
 * - Programmatic navigation
 */

#include "../include/LVPanel.hpp"
#include "../include/LVTileView.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVSlider.hpp"
#include "../include/LVSwitch.hpp"

class TileViewDemoScreen : public LVPanel {
public:
    TileViewDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0xF0F0F0), LV_PART_MAIN);
        createUI();
    }

private:
    LVTileView* tileView = nullptr;
    lv_obj_t* tile1 = nullptr;
    lv_obj_t* tile2 = nullptr;
    lv_obj_t* tile3 = nullptr;
    lv_obj_t* tile4 = nullptr;
    
    void createUI() {
        // Create TileView
        tileView = new LVTileView(this);
        lv_obj_set_size(tileView->obj(), LV_PCT(100), LV_PCT(100));
        
        // Create tiles in 2x2 grid
        // Tile 1 (0,0) - Top Left - Home
        tile1 = tileView->addTile(0, 0, (lv_dir_t)(LV_DIR_RIGHT | LV_DIR_BOTTOM));
        createHomeTile(tile1);
        
        // Tile 2 (1,0) - Top Right - Settings
        tile2 = tileView->addTile(1, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_BOTTOM));
        createSettingsTile(tile2);
        
        // Tile 3 (0,1) - Bottom Left - Chart
        tile3 = tileView->addTile(0, 1, (lv_dir_t)(LV_DIR_RIGHT | LV_DIR_TOP));
        createChartTile(tile3);
        
        // Tile 4 (1,1) - Bottom Right - About
        tile4 = tileView->addTile(1, 1, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_TOP));
        createAboutTile(tile4);
        
        // Set initial tile
        tileView->setTileByIndex(0, 0, LV_ANIM_OFF);
        
        // Tile change callback
        tileView->onTileChange([this](lv_obj_t* tile) {
            // Could update UI based on which tile is active
        });
    }
    
    void createHomeTile(lv_obj_t* tile) {
        lv_obj_set_style_bg_color(tile, lv_color_hex(0xE3F2FD), LV_PART_MAIN);
        
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tile);
        title->setText("Home");
        title->setTextColor(lv_color_hex(0x1976D2));
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_32, 0);
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 40);
        
        auto* welcomeLabel = new LVLabel(nullptr);
        lv_obj_set_parent(welcomeLabel->obj(), tile);
        welcomeLabel->setText(
            "Welcome to TileView Demo!\n\n"
            "Swipe to navigate:\n"
            "• Right: Settings\n"
            "• Down: Chart\n"
            "• Diagonal: About"
        );
        welcomeLabel->setTextColor(lv_color_hex(0x424242));
        lv_obj_set_style_text_align(welcomeLabel->obj(), LV_TEXT_ALIGN_CENTER, 0);
        welcomeLabel->setAlign(LV_ALIGN_CENTER);
        lv_obj_set_y(welcomeLabel->obj(), 20);
        lv_obj_set_style_text_line_space(welcomeLabel->obj(), 8, 0);
        
        // Navigation hint
        auto* hintLabel = new LVLabel(nullptr);
        lv_obj_set_parent(hintLabel->obj(), tile);
        hintLabel->setText(LV_SYMBOL_LEFT " " LV_SYMBOL_RIGHT " " LV_SYMBOL_UP " " LV_SYMBOL_DOWN " Swipe to navigate");
        hintLabel->setTextColor(lv_color_hex(0x757575));
        hintLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(hintLabel->obj(), -20);
    }
    
    void createSettingsTile(lv_obj_t* tile) {
        lv_obj_set_style_bg_color(tile, lv_color_hex(0xF3E5F5), LV_PART_MAIN);
        
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tile);
        title->setText("Settings");
        title->setTextColor(lv_color_hex(0x7B1FA2));
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_32, 0);
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 40);
        
        // Volume slider
        auto* volumeLabel = new LVLabel(nullptr);
        lv_obj_set_parent(volumeLabel->obj(), tile);
        volumeLabel->setText("Volume");
        volumeLabel->setTextColor(lv_color_hex(0x424242));
        lv_obj_set_pos(volumeLabel->obj(), 60, 140);
        
        auto* volumeSlider = new LVSlider(nullptr);
        lv_obj_set_parent(volumeSlider->obj(), tile);
        volumeSlider->setSize(300, 20);
        lv_obj_set_pos(volumeSlider->obj(), 60, 170);
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(70);
        
        // Brightness slider
        auto* brightnessLabel = new LVLabel(nullptr);
        lv_obj_set_parent(brightnessLabel->obj(), tile);
        brightnessLabel->setText("Brightness");
        brightnessLabel->setTextColor(lv_color_hex(0x424242));
        lv_obj_set_pos(brightnessLabel->obj(), 60, 220);
        
        auto* brightnessSlider = new LVSlider(nullptr);
        lv_obj_set_parent(brightnessSlider->obj(), tile);
        brightnessSlider->setSize(300, 20);
        lv_obj_set_pos(brightnessSlider->obj(), 60, 250);
        brightnessSlider->setRange(0, 100);
        brightnessSlider->setValue(85);
        
        // WiFi switch
        auto* wifiLabel = new LVLabel(nullptr);
        lv_obj_set_parent(wifiLabel->obj(), tile);
        wifiLabel->setText("WiFi");
        wifiLabel->setTextColor(lv_color_hex(0x424242));
        lv_obj_set_pos(wifiLabel->obj(), 60, 310);
        
        auto* wifiSwitch = new LVSwitch(nullptr);
        lv_obj_set_parent(wifiSwitch->obj(), tile);
        lv_obj_set_pos(wifiSwitch->obj(), 300, 305);
        wifiSwitch->setOn(true);
        
        // Hint
        auto* hintLabel = new LVLabel(nullptr);
        lv_obj_set_parent(hintLabel->obj(), tile);
        hintLabel->setText(LV_SYMBOL_LEFT " Swipe left to go back");
        hintLabel->setTextColor(lv_color_hex(0x757575));
        hintLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(hintLabel->obj(), -20);
    }
    
    void createChartTile(lv_obj_t* tile) {
        lv_obj_set_style_bg_color(tile, lv_color_hex(0xE8F5E9), LV_PART_MAIN);
        
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tile);
        title->setText("Chart");
        title->setTextColor(lv_color_hex(0x388E3C));
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_32, 0);
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 40);
        
        auto* infoLabel = new LVLabel(nullptr);
        lv_obj_set_parent(infoLabel->obj(), tile);
        infoLabel->setText(
            "Data Visualization\n\n"
            "• Real-time monitoring\n"
            "• Historical trends\n"
            "• Custom charts\n\n"
            "Chart widget would\n"
            "be displayed here"
        );
        infoLabel->setTextColor(lv_color_hex(0x424242));
        lv_obj_set_style_text_align(infoLabel->obj(), LV_TEXT_ALIGN_CENTER, 0);
        infoLabel->setAlign(LV_ALIGN_CENTER);
        lv_obj_set_style_text_line_space(infoLabel->obj(), 8, 0);
        
        // Hint
        auto* hintLabel = new LVLabel(nullptr);
        lv_obj_set_parent(hintLabel->obj(), tile);
        hintLabel->setText(LV_SYMBOL_UP " Swipe up to go back");
        hintLabel->setTextColor(lv_color_hex(0x757575));
        hintLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(hintLabel->obj(), -20);
    }
    
    void createAboutTile(lv_obj_t* tile) {
        lv_obj_set_style_bg_color(tile, lv_color_hex(0xFFF3E0), LV_PART_MAIN);
        
        auto* title = new LVLabel(nullptr);
        lv_obj_set_parent(title->obj(), tile);
        title->setText("About");
        title->setTextColor(lv_color_hex(0xF57C00));
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_32, 0);
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 40);
        
        auto* infoLabel = new LVLabel(nullptr);
        lv_obj_set_parent(infoLabel->obj(), tile);
        infoLabel->setText(
            "TileView Demo\n\n"
            "Version: 1.0.0\n"
            "Build: 2026.01.15\n\n"
            "Device: ESP32-P4\n"
            "LVGL: v9.2.2\n\n"
            "Swipe to navigate\n"
            "between 4 tiles"
        );
        infoLabel->setTextColor(lv_color_hex(0x424242));
        lv_obj_set_style_text_align(infoLabel->obj(), LV_TEXT_ALIGN_CENTER, 0);
        infoLabel->setAlign(LV_ALIGN_CENTER);
        lv_obj_set_style_text_line_space(infoLabel->obj(), 8, 0);
        
        // Hint
        auto* hintLabel = new LVLabel(nullptr);
        lv_obj_set_parent(hintLabel->obj(), tile);
        hintLabel->setText(LV_SYMBOL_LEFT " " LV_SYMBOL_UP " Swipe to navigate");
        hintLabel->setTextColor(lv_color_hex(0x757575));
        hintLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(hintLabel->obj(), -20);
    }
};

extern "C" void create_tileview_demo() {
    auto* screen = new TileViewDemoScreen();
    lv_obj_set_parent(screen->obj(), lv_screen_active());
}
