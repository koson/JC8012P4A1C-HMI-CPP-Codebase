/**
 * @file image_display.cpp
 * @brief Phase 2 Test #13: LVImg - Image Display & Transformation
 * 
 * Test Cases:
 * - TC1: Display LVGL built-in symbols
 * - TC2: Image rotation (0-360 degrees)
 * - TC3: Image zoom (50%-200%)
 * - TC4: Image offset/position
 * - TC5: Multiple images with different transformations
 * - TC6: Real-time rotation animation
 * - TC7: Real-time zoom animation
 * - TC8: Symbol gallery display
 */

#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVImg.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVSlider.hpp"
#include "../include/LVTimer.hpp"
#include "esp_log.h"
#include <cmath>
#include <memory>

static const char* TAG = "IMAGE_TEST";

// Forward declarations
class SymbolGalleryPanel;
class TransformationPanel;
class ImageFeaturesPanel;

// Global pointers for cleanup
static SymbolGalleryPanel* g_symbolGallery = nullptr;
static TransformationPanel* g_transformPanel = nullptr;
static ImageFeaturesPanel* g_featuresPanel = nullptr;

// ==================== Symbol Gallery Panel ====================
class SymbolGalleryPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVImg* images[9];
    LVLabel* labels[9];
    
    struct SymbolInfo {
        const char* symbol;
        const char* name;
    };
    
    SymbolInfo symbols[9] = {
        {LV_SYMBOL_AUDIO, "Audio"},
        {LV_SYMBOL_VIDEO, "Video"},
        {LV_SYMBOL_LIST, "List"},
        {LV_SYMBOL_OK, "OK"},
        {LV_SYMBOL_CLOSE, "Close"},
        {LV_SYMBOL_POWER, "Power"},
        {LV_SYMBOL_SETTINGS, "Settings"},
        {LV_SYMBOL_HOME, "Home"},
        {LV_SYMBOL_DOWNLOAD, "Download"}
    };

public:
    SymbolGalleryPanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 380);
        container->setPos(20, 20);
        container->setBackgroundColor(lv_color_hex(0x1E2A3A));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x3498DB));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 15, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Symbol Gallery");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Create 3x3 grid of symbols
        for (int i = 0; i < 9; i++) {
            int row = i / 3;
            int col = i % 3;
            
            // Symbol image
            images[i] = new LVImg(container);
            images[i]->setSymbol(symbols[i].symbol);
            images[i]->setAlign(LV_ALIGN_TOP_LEFT);
            images[i]->setPos(60 + col * 140, 60 + row * 100);
            
            // Style the symbol
            lv_obj_set_style_text_color(images[i]->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
            lv_obj_set_style_text_font(images[i]->obj(), &lv_font_montserrat_48, LV_PART_MAIN);
            
            // Symbol name label
            labels[i] = new LVLabel(container);
            labels[i]->setText(symbols[i].name);
            labels[i]->setTextColor(lv_color_hex(0xBDC3C7));
            labels[i]->setFont(&lv_font_montserrat_14);
            labels[i]->setAlign(LV_ALIGN_TOP_LEFT);
            labels[i]->setPos(60 + col * 140, 110 + row * 100);
        }
        
        ESP_LOGI(TAG, "✅ Symbol Gallery created");
        ESP_LOGI(TAG, "   - 9 LVGL built-in symbols displayed");
        ESP_LOGI(TAG, "   - 3x3 grid layout");
    }
};

// ==================== Transformation Panel ====================
class TransformationPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVImg* centerImage;
    LVSlider* rotationSlider;
    LVSlider* zoomSlider;
    LVLabel* rotationLabel;
    LVLabel* zoomLabel;
    LVButton* btnReset;
    
    int16_t currentRotation = 0;
    uint16_t currentZoom = 256;  // 256 = 100%

    void updateRotationLabel() {
        char buf[32];
        snprintf(buf, sizeof(buf), "Rotation: %d°", currentRotation / 10);
        rotationLabel->setText(buf);
    }
    
    void updateZoomLabel() {
        char buf[32];
        snprintf(buf, sizeof(buf), "Zoom: %d%%", (currentZoom * 100) / 256);
        zoomLabel->setText(buf);
    }

public:
    TransformationPanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 380);
        container->setPos(520, 20);
        container->setBackgroundColor(lv_color_hex(0x2C3E50));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0xE74C3C));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 15, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Image Transform");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Center image (Settings symbol)
        centerImage = new LVImg(container);
        centerImage->setSymbol(LV_SYMBOL_SETTINGS);
        centerImage->setAlign(LV_ALIGN_TOP_MID);
        centerImage->setPos(0, 90);
        lv_obj_set_style_text_color(centerImage->obj(), lv_color_hex(0xE74C3C), LV_PART_MAIN);
        lv_obj_set_style_text_font(centerImage->obj(), &lv_font_montserrat_48, LV_PART_MAIN);
        
        // Set pivot to center
        centerImage->setPivot(24, 24);  // Half of 48px font
        
        // Rotation slider
        rotationLabel = new LVLabel(container);
        rotationLabel->setText("Rotation: 0°");
        rotationLabel->setTextColor(lv_color_hex(0xECF0F1));
        rotationLabel->setFont(&lv_font_montserrat_16);
        rotationLabel->setPos(20, 190);
        
        rotationSlider = new LVSlider(container);
        rotationSlider->setSize(300, 15);
        rotationSlider->setPos(20, 220);
        rotationSlider->setRange(0, 3600);  // 0-360 degrees * 10
        rotationSlider->setValue(0);
        rotationSlider->setBackgroundColor(lv_color_hex(0x34495E));
        lv_obj_set_style_bg_color(rotationSlider->obj(), lv_color_hex(0xE74C3C), LV_PART_INDICATOR);
        rotationSlider->onValueChanged([this](LVWidget* w, lv_event_t* e) {
            currentRotation = rotationSlider->getValue();
            centerImage->setAngle(currentRotation);
            updateRotationLabel();
            ESP_LOGI(TAG, "Rotation: %d°", currentRotation / 10);
        });
        
        // Zoom slider
        zoomLabel = new LVLabel(container);
        zoomLabel->setText("Zoom: 100%");
        zoomLabel->setTextColor(lv_color_hex(0xECF0F1));
        zoomLabel->setFont(&lv_font_montserrat_16);
        zoomLabel->setPos(20, 250);
        
        zoomSlider = new LVSlider(container);
        zoomSlider->setSize(300, 15);
        zoomSlider->setPos(20, 280);
        zoomSlider->setRange(128, 512);  // 50%-200%
        zoomSlider->setValue(256);  // 100%
        zoomSlider->setBackgroundColor(lv_color_hex(0x34495E));
        lv_obj_set_style_bg_color(zoomSlider->obj(), lv_color_hex(0x27AE60), LV_PART_INDICATOR);
        zoomSlider->onValueChanged([this](LVWidget* w, lv_event_t* e) {
            currentZoom = zoomSlider->getValue();
            centerImage->setZoom(currentZoom);
            updateZoomLabel();
            ESP_LOGI(TAG, "Zoom: %d%%", (currentZoom * 100) / 256);
        });
        
        // Reset button
        btnReset = new LVButton(container);
        btnReset->setSize(150, 50);
        btnReset->setAlign(LV_ALIGN_BOTTOM_MID);
        btnReset->setPos(0, -10);
        btnReset->setBackgroundColor(lv_color_hex(0xE67E22));
        btnReset->setRadius(25);
        btnReset->setText("Reset");
        btnReset->setTextColor(lv_color_hex(0xFFFFFF));
        btnReset->setFont(&lv_font_montserrat_18);
        btnReset->onClicked([this](LVWidget* w, lv_event_t* e) {
            rotationSlider->setValue(0);
            zoomSlider->setValue(256);
            currentRotation = 0;
            currentZoom = 256;
            centerImage->setAngle(0);
            centerImage->setZoom(256);
            updateRotationLabel();
            updateZoomLabel();
            ESP_LOGI(TAG, "Reset to default");
        });
        
        ESP_LOGI(TAG, "✅ Transformation Panel created");
        ESP_LOGI(TAG, "   - Rotation: 0-360°");
        ESP_LOGI(TAG, "   - Zoom: 50%-200%%");
        ESP_LOGI(TAG, "   - Real-time preview");
    }
};

// ==================== Image Features Panel ====================
class ImageFeaturesPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVImg* rotatingImage;
    LVImg* scalingImage;
    LVImg* staticImage;
    LVLabel* statusLabel;
    
    uint32_t animationStep = 0;
    std::unique_ptr<LVTimer> animTimer;
    
    void updateAnimation() {
        animationStep += 10;
        
        // Rotating image: 0-360 degrees
        int16_t rotation = (animationStep * 10) % 3600;
        rotatingImage->setAngle(rotation);
        
        // Scaling image: 50%-150% sine wave
        float scale = 1.0f + 0.5f * std::sin(animationStep * 0.02f);
        uint16_t zoom = static_cast<uint16_t>(scale * 256);
        scalingImage->setZoom(zoom);
        
        // Update status
        char buf[64];
        snprintf(buf, sizeof(buf), "R:%d° Z:%d%%", rotation / 10, (zoom * 100) / 256);
        statusLabel->setText(buf);
    }

public:
    ImageFeaturesPanel(LVWidget* parent) {
        // Container
        container = new LVPanel(parent);
        container->setSize(480, 180);
        container->setPos(20, 420);
        container->setBackgroundColor(lv_color_hex(0x16213E));
        container->setBorderWidth(2);
        container->setBorderColor(lv_color_hex(0x27AE60));
        container->setRadius(15);
        lv_obj_set_style_pad_all(container->obj(), 15, LV_PART_MAIN);
        
        // Title
        titleLabel = new LVLabel(container);
        titleLabel->setText("Animated Features");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_20);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Static image
        staticImage = new LVImg(container);
        staticImage->setSymbol(LV_SYMBOL_HOME);
        staticImage->setAlign(LV_ALIGN_TOP_LEFT);
        staticImage->setPos(60, 60);
        lv_obj_set_style_text_color(staticImage->obj(), lv_color_hex(0x95A5A6), LV_PART_MAIN);
        lv_obj_set_style_text_font(staticImage->obj(), &lv_font_montserrat_48, LV_PART_MAIN);
        
        // Rotating image
        rotatingImage = new LVImg(container);
        rotatingImage->setSymbol(LV_SYMBOL_REFRESH);
        rotatingImage->setAlign(LV_ALIGN_TOP_LEFT);
        rotatingImage->setPos(220, 60);
        rotatingImage->setPivot(24, 24);
        lv_obj_set_style_text_color(rotatingImage->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_text_font(rotatingImage->obj(), &lv_font_montserrat_48, LV_PART_MAIN);
        
        // Scaling image
        scalingImage = new LVImg(container);
        scalingImage->setSymbol(LV_SYMBOL_WARNING);
        scalingImage->setAlign(LV_ALIGN_TOP_LEFT);
        scalingImage->setPos(380, 60);
        lv_obj_set_style_text_color(scalingImage->obj(), lv_color_hex(0xF39C12), LV_PART_MAIN);
        lv_obj_set_style_text_font(scalingImage->obj(), &lv_font_montserrat_48, LV_PART_MAIN);
        
        // Status label
        statusLabel = new LVLabel(container);
        statusLabel->setText("R:0° Z:100%");
        statusLabel->setTextColor(lv_color_hex(0x27AE60));
        statusLabel->setFont(&lv_font_montserrat_16);
        statusLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        statusLabel->setPos(0, -5);
        
        // Start animation timer
        animTimer = std::make_unique<LVTimer>(50, [this]() {
            updateAnimation();
        });
        
        ESP_LOGI(TAG, "✅ Animated Features created");
        ESP_LOGI(TAG, "   - Rotating icon (360°)");
        ESP_LOGI(TAG, "   - Scaling icon (50-150%%)");
        ESP_LOGI(TAG, "   - Animation: 50ms refresh");
    }
};

// ==================== Main Test Function ====================
extern "C" void test_image_display() {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #13: LVImg - Image Display");
    ESP_LOGI(TAG, "========================================");
    
    lv_obj_t* screen = lv_scr_act();
    lv_obj_clean(screen);
    
    // Set dark background
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0A0E1A), LV_PART_MAIN);
    
    // Create panels
    g_symbolGallery = new SymbolGalleryPanel(nullptr);
    g_transformPanel = new TransformationPanel(nullptr);
    g_featuresPanel = new ImageFeaturesPanel(nullptr);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Test Cases:");
    ESP_LOGI(TAG, "✅ TC1: Built-in symbols displayed (9 symbols)");
    ESP_LOGI(TAG, "✅ TC2: Interactive rotation control (0-360°)");
    ESP_LOGI(TAG, "✅ TC3: Interactive zoom control (50-200%%)");
    ESP_LOGI(TAG, "✅ TC4: Pivot point transformation");
    ESP_LOGI(TAG, "✅ TC5: Multiple images independently styled");
    ESP_LOGI(TAG, "✅ TC6: Real-time rotation animation");
    ESP_LOGI(TAG, "✅ TC7: Real-time zoom animation");
    ESP_LOGI(TAG, "✅ TC8: Symbol gallery with labels");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Instructions:");
    ESP_LOGI(TAG, "- Drag rotation slider to rotate center image");
    ESP_LOGI(TAG, "- Drag zoom slider to scale center image");
    ESP_LOGI(TAG, "- Press Reset button to restore defaults");
    ESP_LOGI(TAG, "- Watch bottom panel for auto-animation");
    ESP_LOGI(TAG, "========================================");
}

extern "C" void cleanup_image_display() {
    ESP_LOGI(TAG, "Cleaning up image display demo...");
    
    // Delete panels (this will also delete their timers via smart pointers)
    if (g_featuresPanel) {
        delete g_featuresPanel;
        g_featuresPanel = nullptr;
    }
    if (g_transformPanel) {
        delete g_transformPanel;
        g_transformPanel = nullptr;
    }
    if (g_symbolGallery) {
        delete g_symbolGallery;
        g_symbolGallery = nullptr;
    }
    
    ESP_LOGI(TAG, "Image display cleanup complete");
}
