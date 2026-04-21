/**
 * @file colorwheel_demo.cpp
 * @brief Demo for LVColorWheel - Color picker widget testing
 * 
 * Tests:
 * - Color wheel with hue selection
 * - Real-time color preview
 * - RGB/HSV value display
 * - Color change callbacks
 */

#include "../include/LVPanel.hpp"
#include "../include/LVColorWheel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVSlider.hpp"
#include "esp_log.h"

class ColorWheelDemoScreen : public LVPanel {
public:
    ColorWheelDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0x1E1E1E), LV_PART_MAIN);
        createUI();
    }

private:
    LVColorWheel* colorWheel = nullptr;
    LVPanel* colorPreview = nullptr;
    LVLabel* hueLabel = nullptr;
    LVLabel* rgbLabel = nullptr;
    LVLabel* hexLabel = nullptr;
    LVSlider* satSlider = nullptr;
    LVSlider* valSlider = nullptr;
    
    uint8_t currentSat = 100;
    uint8_t currentVal = 100;
    
    void createUI() {
        // Title
        auto* title = new LVLabel(this);
        title->setText("Color Picker Demo");
        title->setTextColor(lv_color_hex(0xFFFFFF));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 10);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_24, 0);
        
        // Color wheel (Hue selector)
        colorWheel = new LVColorWheel(this, true);
        colorWheel->setSize(250, 250);
        colorWheel->setPos(50, 80);
        
        // Color change callback
        colorWheel->onColorChange([this](lv_color_t color) {
            updateColorInfo(color);
        });
        
        // Color preview panel
        colorPreview = new LVPanel(this);
        colorPreview->setSize(200, 200);
        colorPreview->setPos(350, 80);
        lv_obj_set_style_bg_color(colorPreview->obj(), lv_color_hex(0xFF0000), LV_PART_MAIN);
        lv_obj_set_style_border_width(colorPreview->obj(), 3, LV_PART_MAIN);
        lv_obj_set_style_border_color(colorPreview->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        
        // Preview label
        auto* previewLabel = new LVLabel(this);
        previewLabel->setText("Color Preview");
        previewLabel->setTextColor(lv_color_hex(0xCCCCCC));
        previewLabel->setPos(380, 290);
        
        // Saturation slider
        auto* satLabel = new LVLabel(this);
        satLabel->setText("Saturation");
        satLabel->setTextColor(lv_color_hex(0xFFFFFF));
        satLabel->setPos(600, 80);
        
        satSlider = new LVSlider(this);
        satSlider->setSize(300, 15);
        satSlider->setPos(600, 110);
        satSlider->setRange(0, 100);
        satSlider->setValue(100);
        
        satSlider->onChange([this](int32_t value) {
            currentSat = value;
            updateColorPreview();
        });
        
        // Value/Brightness slider
        auto* valLabel = new LVLabel(this);
        valLabel->setText("Brightness");
        valLabel->setTextColor(lv_color_hex(0xFFFFFF));
        valLabel->setPos(600, 150);
        
        valSlider = new LVSlider(this);
        valSlider->setSize(300, 15);
        valSlider->setPos(600, 180);
        valSlider->setRange(0, 100);
        valSlider->setValue(100);
        
        valSlider->onChange([this](int32_t value) {
            currentVal = value;
            updateColorPreview();
        });
        
        // Color info labels
        hueLabel = new LVLabel(this);
        hueLabel->setText("Hue: 0°");
        hueLabel->setTextColor(lv_color_hex(0xFFFFFF));
        hueLabel->setPos(600, 230);
        
        rgbLabel = new LVLabel(this);
        rgbLabel->setText("RGB: (255, 0, 0)");
        rgbLabel->setTextColor(lv_color_hex(0xFFFFFF));
        rgbLabel->setPos(600, 260);
        
        hexLabel = new LVLabel(this);
        hexLabel->setText("HEX: #FF0000");
        hexLabel->setTextColor(lv_color_hex(0xFFFFFF));
        hexLabel->setPos(600, 290);
        
        // Preset color buttons
        auto* presetsLabel = new LVLabel(this);
        presetsLabel->setText("Preset Colors:");
        presetsLabel->setTextColor(lv_color_hex(0xCCCCCC));
        presetsLabel->setPos(50, 350);
        
        createPresetButton(50, 380, 0xFF0000, "Red");
        createPresetButton(150, 380, 0x00FF00, "Green");
        createPresetButton(250, 380, 0x0000FF, "Blue");
        createPresetButton(350, 380, 0xFFFF00, "Yellow");
        createPresetButton(450, 380, 0xFF00FF, "Magenta");
        createPresetButton(550, 380, 0x00FFFF, "Cyan");
        createPresetButton(650, 380, 0xFFA500, "Orange");
        createPresetButton(750, 380, 0x800080, "Purple");
        
        // Reset button
        auto* resetBtn = new LVButton(this);
        resetBtn->setSize(150, 45);
        resetBtn->setPos(50, 450);
        resetBtn->setText("Reset");
        resetBtn->onClicked([this](LVWidget* w, lv_event_t* e) {
            colorWheel->setHue(0);
            satSlider->setValue(100);
            valSlider->setValue(100);
            currentSat = 100;
            currentVal = 100;
            updateColorPreview();
            ESP_LOGI("ColorWheelDemo", "Color reset to red");
        });
        
        // Initial color update
        updateColorInfo(lv_color_hex(0xFF0000));
        
        ESP_LOGI("ColorWheelDemo", "Color wheel demo initialized");
    }
    
    void createPresetButton(int x, int y, uint32_t color, const char* name) {
        auto* btn = new LVButton(this);
        btn->setSize(80, 40);
        btn->setPos(x, y);
        lv_obj_set_style_bg_color(btn->obj(), lv_color_hex(color), LV_PART_MAIN);
        
        btn->onClicked([this, color, name](LVWidget* w, lv_event_t* e) {
            // Convert RGB to HSV to set hue
            lv_color_t c = lv_color_hex(color);
            lv_color_hsv_t hsv = lv_color_rgb_to_hsv(c.red, c.green, c.blue);
            
            colorWheel->setHue(hsv.h);
            satSlider->setValue(hsv.s);
            valSlider->setValue(hsv.v);
            currentSat = hsv.s;
            currentVal = hsv.v;
            updateColorPreview();
            
            ESP_LOGI("ColorWheelDemo", "Preset color selected: %s (0x%06X)", name, (unsigned int)color);
        });
    }
    
    void updateColorInfo(lv_color_t color) {
        // Get HSV values
        lv_color_hsv_t hsv = colorWheel->getColorHSV();
        
        // Update hue label
        char buf[64];
        snprintf(buf, sizeof(buf), "Hue: %d°", hsv.h);
        hueLabel->setText(buf);
        
        updateColorPreview();
    }
    
    void updateColorPreview() {
        // Get current hue from wheel
        uint16_t hue = colorWheel->getHue();
        
        // Create HSV color with current saturation and value
        lv_color_hsv_t hsv = {hue, currentSat, currentVal};
        lv_color_t color = lv_color_hsv_to_rgb(hsv.h, hsv.s, hsv.v);
        
        // Update preview
        lv_obj_set_style_bg_color(colorPreview->obj(), color, LV_PART_MAIN);
        
        // Update RGB label
        char buf[64];
        snprintf(buf, sizeof(buf), "RGB: (%d, %d, %d)", 
                 color.red, color.green, color.blue);
        rgbLabel->setText(buf);
        
        // Update HEX label
        uint32_t hex = (color.red << 16) | (color.green << 8) | color.blue;
        snprintf(buf, sizeof(buf), "HEX: #%06X", (unsigned int)hex);
        hexLabel->setText(buf);
    }
};

extern "C" void create_colorwheel_demo() {
    auto* screen = lv_screen_active();
    auto* panel = new ColorWheelDemoScreen();
    lv_obj_set_parent(panel->obj(), screen);
}
