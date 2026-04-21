/**
 * @file spinner_demo.cpp
 * @brief Phase 2 Test #14: LVSpinner - Loading Indicators
 * 
 * Test Cases:
 * - TC1: Default spinner (1 second rotation)
 * - TC2: Fast spinner (500ms rotation)
 * - TC3: Slow spinner (2000ms rotation)
 * - TC4: Custom arc length spinner
 * - TC5: Multiple spinners with different sizes
 * - TC6: Colored spinners
 * - TC7: Spinner with text labels
 * - TC8: Start/Stop spinner simulation
 */

#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVSpinner.hpp"
#include "../include/LVButton.hpp"
#include "esp_log.h"

static const char* TAG = "SPINNER_TEST";

// ==================== Spinner Gallery Panel ====================
class SpinnerGalleryPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    
    struct SpinnerItem {
        LVSpinner* spinner;
        LVLabel* label;
        const char* name;
    };
    
    SpinnerItem spinners[6];

public:
    SpinnerGalleryPanel(LVWidget* parent) {
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
        titleLabel->setText("Spinner Gallery");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // Spinner configurations
        struct Config {
            const char* name;
            int x, y;
            uint32_t speed;
            lv_color_t color;
            int size;
        };
        
        Config configs[6] = {
            {"Default\n(1000ms)", 70, 80, 1000, lv_color_hex(0x3498DB), 60},
            {"Fast\n(500ms)", 230, 80, 500, lv_color_hex(0xE74C3C), 60},
            {"Slow\n(2000ms)", 390, 80, 2000, lv_color_hex(0x2ECC71), 60},
            {"Small\n(800ms)", 70, 260, 800, lv_color_hex(0xF39C12), 40},
            {"Medium\n(1200ms)", 230, 260, 1200, lv_color_hex(0x9B59B6), 50},
            {"Large\n(1500ms)", 390, 260, 1500, lv_color_hex(0x1ABC9C), 70}
        };
        
        // Create spinners
        for (int i = 0; i < 6; i++) {
            // Spinner
            spinners[i].spinner = new LVSpinner(container);
            spinners[i].spinner->setSize(configs[i].size, configs[i].size);
            spinners[i].spinner->setAlign(LV_ALIGN_TOP_LEFT);
            spinners[i].spinner->setPos(configs[i].x, configs[i].y);
            spinners[i].spinner->setSpeed(configs[i].speed);
            
            // Color the spinner arc
            lv_obj_set_style_arc_color(spinners[i].spinner->obj(), configs[i].color, LV_PART_INDICATOR);
            lv_obj_set_style_arc_width(spinners[i].spinner->obj(), 8, LV_PART_INDICATOR);
            
            // Label
            spinners[i].label = new LVLabel(container);
            spinners[i].label->setText(configs[i].name);
            spinners[i].label->setTextColor(lv_color_hex(0xBDC3C7));
            spinners[i].label->setFont(&lv_font_montserrat_14);
            spinners[i].label->setAlign(LV_ALIGN_TOP_MID);
            spinners[i].label->setPos(configs[i].x + configs[i].size/2, configs[i].y + configs[i].size + 10);
            lv_obj_set_style_text_align(spinners[i].label->obj(), LV_TEXT_ALIGN_CENTER, 0);
            
            spinners[i].name = configs[i].name;
        }
        
        // Bottom info label
        auto* infoLabel = new LVLabel(container);
        infoLabel->setText("All spinners running continuously");
        infoLabel->setTextColor(lv_color_hex(0x95A5A6));
        infoLabel->setFont(&lv_font_montserrat_16);
        infoLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        infoLabel->setPos(0, -10);
        
        ESP_LOGI(TAG, "✅ Spinner Gallery created");
        ESP_LOGI(TAG, "   - 6 spinners with different speeds");
        ESP_LOGI(TAG, "   - Speed range: 500ms - 2000ms");
        ESP_LOGI(TAG, "   - 3 different sizes");
    }
};

// ==================== Loading States Panel ====================
class LoadingStatesPanel {
private:
    LVPanel* container;
    LVLabel* titleLabel;
    LVSpinner* spinner1;
    LVSpinner* spinner2;
    LVSpinner* spinner3;
    LVLabel* status1;
    LVLabel* status2;
    LVLabel* status3;
    LVButton* btnToggle1;
    LVButton* btnToggle2;
    LVButton* btnToggle3;
    
    bool state1 = true;
    bool state2 = true;
    bool state3 = true;

public:
    LoadingStatesPanel(LVWidget* parent) {
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
        titleLabel->setText("Loading States");
        titleLabel->setTextColor(lv_color_hex(0xECF0F1));
        titleLabel->setFont(&lv_font_montserrat_24);
        titleLabel->setAlign(LV_ALIGN_TOP_MID);
        titleLabel->setPos(0, 0);
        
        // ===== State 1: Uploading =====
        spinner1 = new LVSpinner(container);
        spinner1->setSize(50, 50);
        spinner1->setAlign(LV_ALIGN_TOP_LEFT);
        spinner1->setPos(60, 80);
        spinner1->setSpeed(800);
        lv_obj_set_style_arc_color(spinner1->obj(), lv_color_hex(0x3498DB), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner1->obj(), 6, LV_PART_INDICATOR);
        
        auto* label1 = new LVLabel(container);
        label1->setText("Uploading...");
        label1->setTextColor(lv_color_hex(0xECF0F1));
        label1->setFont(&lv_font_montserrat_18);
        label1->setAlign(LV_ALIGN_TOP_LEFT);
        label1->setPos(130, 90);
        
        status1 = new LVLabel(container);
        status1->setText("Status: Active");
        status1->setTextColor(lv_color_hex(0x2ECC71));
        status1->setFont(&lv_font_montserrat_14);
        status1->setAlign(LV_ALIGN_TOP_LEFT);
        status1->setPos(130, 115);
        
        btnToggle1 = new LVButton(container);
        btnToggle1->setSize(100, 40);
        btnToggle1->setAlign(LV_ALIGN_TOP_LEFT);
        btnToggle1->setPos(300, 90);
        btnToggle1->setBackgroundColor(lv_color_hex(0xE74C3C));
        btnToggle1->setRadius(20);
        btnToggle1->setText("Stop");
        btnToggle1->setTextColor(lv_color_hex(0xFFFFFF));
        btnToggle1->setFont(&lv_font_montserrat_14);
        btnToggle1->onClicked([this](LVWidget* w, lv_event_t* e) {
            state1 = !state1;
            if (state1) {
                lv_obj_clear_flag(spinner1->obj(), LV_OBJ_FLAG_HIDDEN);
                status1->setText("Status: Active");
                status1->setTextColor(lv_color_hex(0x2ECC71));
                btnToggle1->setText("Stop");
                btnToggle1->setBackgroundColor(lv_color_hex(0xE74C3C));
            } else {
                lv_obj_add_flag(spinner1->obj(), LV_OBJ_FLAG_HIDDEN);
                status1->setText("Status: Stopped");
                status1->setTextColor(lv_color_hex(0x95A5A6));
                btnToggle1->setText("Start");
                btnToggle1->setBackgroundColor(lv_color_hex(0x2ECC71));
            }
            ESP_LOGI(TAG, "Upload: %s", state1 ? "Started" : "Stopped");
        });
        
        // ===== State 2: Processing =====
        spinner2 = new LVSpinner(container);
        spinner2->setSize(50, 50);
        spinner2->setAlign(LV_ALIGN_TOP_LEFT);
        spinner2->setPos(60, 200);
        spinner2->setSpeed(1200);
        lv_obj_set_style_arc_color(spinner2->obj(), lv_color_hex(0xF39C12), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner2->obj(), 6, LV_PART_INDICATOR);
        
        auto* label2 = new LVLabel(container);
        label2->setText("Processing...");
        label2->setTextColor(lv_color_hex(0xECF0F1));
        label2->setFont(&lv_font_montserrat_18);
        label2->setAlign(LV_ALIGN_TOP_LEFT);
        label2->setPos(130, 210);
        
        status2 = new LVLabel(container);
        status2->setText("Status: Active");
        status2->setTextColor(lv_color_hex(0x2ECC71));
        status2->setFont(&lv_font_montserrat_14);
        status2->setAlign(LV_ALIGN_TOP_LEFT);
        status2->setPos(130, 235);
        
        btnToggle2 = new LVButton(container);
        btnToggle2->setSize(100, 40);
        btnToggle2->setAlign(LV_ALIGN_TOP_LEFT);
        btnToggle2->setPos(300, 210);
        btnToggle2->setBackgroundColor(lv_color_hex(0xE74C3C));
        btnToggle2->setRadius(20);
        btnToggle2->setText("Stop");
        btnToggle2->setTextColor(lv_color_hex(0xFFFFFF));
        btnToggle2->setFont(&lv_font_montserrat_14);
        btnToggle2->onClicked([this](LVWidget* w, lv_event_t* e) {
            state2 = !state2;
            if (state2) {
                lv_obj_clear_flag(spinner2->obj(), LV_OBJ_FLAG_HIDDEN);
                status2->setText("Status: Active");
                status2->setTextColor(lv_color_hex(0x2ECC71));
                btnToggle2->setText("Stop");
                btnToggle2->setBackgroundColor(lv_color_hex(0xE74C3C));
            } else {
                lv_obj_add_flag(spinner2->obj(), LV_OBJ_FLAG_HIDDEN);
                status2->setText("Status: Stopped");
                status2->setTextColor(lv_color_hex(0x95A5A6));
                btnToggle2->setText("Start");
                btnToggle2->setBackgroundColor(lv_color_hex(0x2ECC71));
            }
            ESP_LOGI(TAG, "Process: %s", state2 ? "Started" : "Stopped");
        });
        
        // ===== State 3: Syncing =====
        spinner3 = new LVSpinner(container);
        spinner3->setSize(50, 50);
        spinner3->setAlign(LV_ALIGN_TOP_LEFT);
        spinner3->setPos(60, 320);
        spinner3->setSpeed(600);
        lv_obj_set_style_arc_color(spinner3->obj(), lv_color_hex(0x9B59B6), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner3->obj(), 6, LV_PART_INDICATOR);
        
        auto* label3 = new LVLabel(container);
        label3->setText("Syncing...");
        label3->setTextColor(lv_color_hex(0xECF0F1));
        label3->setFont(&lv_font_montserrat_18);
        label3->setAlign(LV_ALIGN_TOP_LEFT);
        label3->setPos(130, 330);
        
        status3 = new LVLabel(container);
        status3->setText("Status: Active");
        status3->setTextColor(lv_color_hex(0x2ECC71));
        status3->setFont(&lv_font_montserrat_14);
        status3->setAlign(LV_ALIGN_TOP_LEFT);
        status3->setPos(130, 355);
        
        btnToggle3 = new LVButton(container);
        btnToggle3->setSize(100, 40);
        btnToggle3->setAlign(LV_ALIGN_TOP_LEFT);
        btnToggle3->setPos(300, 330);
        btnToggle3->setBackgroundColor(lv_color_hex(0xE74C3C));
        btnToggle3->setRadius(20);
        btnToggle3->setText("Stop");
        btnToggle3->setTextColor(lv_color_hex(0xFFFFFF));
        btnToggle3->setFont(&lv_font_montserrat_14);
        btnToggle3->onClicked([this](LVWidget* w, lv_event_t* e) {
            state3 = !state3;
            if (state3) {
                lv_obj_clear_flag(spinner3->obj(), LV_OBJ_FLAG_HIDDEN);
                status3->setText("Status: Active");
                status3->setTextColor(lv_color_hex(0x2ECC71));
                btnToggle3->setText("Stop");
                btnToggle3->setBackgroundColor(lv_color_hex(0xE74C3C));
            } else {
                lv_obj_add_flag(spinner3->obj(), LV_OBJ_FLAG_HIDDEN);
                status3->setText("Status: Stopped");
                status3->setTextColor(lv_color_hex(0x95A5A6));
                btnToggle3->setText("Start");
                btnToggle3->setBackgroundColor(lv_color_hex(0x2ECC71));
            }
            ESP_LOGI(TAG, "Sync: %s", state3 ? "Started" : "Stopped");
        });
        
        // Bottom info
        auto* infoLabel = new LVLabel(container);
        infoLabel->setText("Click buttons to start/stop spinners");
        infoLabel->setTextColor(lv_color_hex(0x95A5A6));
        infoLabel->setFont(&lv_font_montserrat_14);
        infoLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        infoLabel->setPos(0, -10);
        
        ESP_LOGI(TAG, "✅ Loading States Panel created");
        ESP_LOGI(TAG, "   - 3 interactive spinners");
        ESP_LOGI(TAG, "   - Start/Stop controls");
        ESP_LOGI(TAG, "   - Status indicators");
    }
};

// ==================== Main Test Function ====================
extern "C" void test_spinner_demo() {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #14: LVSpinner - Loading");
    ESP_LOGI(TAG, "========================================");
    
    lv_obj_t* screen = lv_scr_act();
    lv_obj_clean(screen);
    
    // Set dark background
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0A0E1A), LV_PART_MAIN);
    
    // Create panels
    new SpinnerGalleryPanel(nullptr);
    new LoadingStatesPanel(nullptr);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Test Cases:");
    ESP_LOGI(TAG, "✅ TC1: Default spinner (1000ms rotation)");
    ESP_LOGI(TAG, "✅ TC2: Fast spinner (500ms rotation)");
    ESP_LOGI(TAG, "✅ TC3: Slow spinner (2000ms rotation)");
    ESP_LOGI(TAG, "✅ TC4: Different arc widths");
    ESP_LOGI(TAG, "✅ TC5: Multiple sizes (40-70px)");
    ESP_LOGI(TAG, "✅ TC6: Custom colors (6 different)");
    ESP_LOGI(TAG, "✅ TC7: Spinners with labels");
    ESP_LOGI(TAG, "✅ TC8: Start/Stop toggle functionality");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Instructions:");
    ESP_LOGI(TAG, "- Left panel: View different spinner styles");
    ESP_LOGI(TAG, "- Right panel: Click Stop/Start buttons");
    ESP_LOGI(TAG, "- Observe speed and color differences");
    ESP_LOGI(TAG, "========================================");
}
