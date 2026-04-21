#include "lvgl.h"
#include "esp_log.h"
#include "LVTimer.hpp"
#include "LVPanel.hpp"
#include "LVLabel.hpp"
#include "LVButton.hpp"
#include "LVArc.hpp"
#include <ctime>
#include <memory>

static const char* TAG = "TIMER_TEST";

// Global variables for timer demo - use smart pointers for automatic cleanup
static std::unique_ptr<LVTimer> blinkTimer;
static std::unique_ptr<LVTimer> clockTimer;
static std::unique_ptr<LVTimer> countdownTimer;
static std::unique_ptr<LVTimer> arcTimer;

static LVLabel* blinkLabel = nullptr;
static LVLabel* clockLabel = nullptr;
static LVLabel* countdownLabel = nullptr;
static LVLabel* statusLabel = nullptr;
static LVArc* progressArc = nullptr;

static int blinkState = 0;
static int countdownValue = 10;
static int arcValue = 0;

extern "C" void test_timer_demo()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 3 Test #1: LVTimer - Timer Demo");
    ESP_LOGI(TAG, "========================================");
    
    // Main container
    LVPanel* container = new LVPanel(nullptr);
    container->setSize(1024, 600);
    lv_obj_set_style_bg_color(container->obj(), lv_color_hex(0x1a1a2e), LV_PART_MAIN);
    lv_obj_set_style_pad_all(container->obj(), 20, LV_PART_MAIN);
    
    // Title
    LVLabel* titleLabel = new LVLabel(container);
    titleLabel->setText(LV_SYMBOL_REFRESH " LVTimer Demo - 4 Timer Examples");
    lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(titleLabel->obj(), lv_color_hex(0xffffff), LV_PART_MAIN);
    titleLabel->setAlign(LV_ALIGN_TOP_MID);
    titleLabel->setPos(0, 10);
    
    // === DEMO 1: Blinking Indicator (500ms) ===
    LVPanel* blinkPanel = new LVPanel(container);
    blinkPanel->setSize(480, 120);
    lv_obj_set_style_bg_color(blinkPanel->obj(), lv_color_hex(0x2c3e50), LV_PART_MAIN);
    lv_obj_set_style_radius(blinkPanel->obj(), 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(blinkPanel->obj(), 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(blinkPanel->obj(), lv_color_hex(0x3498db), LV_PART_MAIN);
    blinkPanel->setPos(20, 70);
    
    LVLabel* blinkTitle = new LVLabel(blinkPanel);
    blinkTitle->setText("[1] Blink Timer (500ms)");
    lv_obj_set_style_text_color(blinkTitle->obj(), lv_color_hex(0x3498db), LV_PART_MAIN);
    blinkTitle->setAlign(LV_ALIGN_TOP_LEFT);
    blinkTitle->setPos(15, 10);
    
    blinkLabel = new LVLabel(blinkPanel);
    blinkLabel->setText(LV_SYMBOL_BLUETOOTH " ON");
    lv_obj_set_style_text_font(blinkLabel->obj(), &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(blinkLabel->obj(), lv_color_hex(0x3498db), LV_PART_MAIN);
    blinkLabel->setAlign(LV_ALIGN_CENTER);
    blinkLabel->setPos(0, 10);
    
    // Blink timer: toggle every 500ms
    blinkTimer = std::make_unique<LVTimer>(500, []() {
        blinkState = !blinkState;
        if (blinkState) {
            blinkLabel->setText(LV_SYMBOL_BLUETOOTH " ON");
            lv_obj_set_style_text_color(blinkLabel->obj(), lv_color_hex(0x3498db), LV_PART_MAIN);
        } else {
            blinkLabel->setText(LV_SYMBOL_CLOSE " OFF");
            lv_obj_set_style_text_color(blinkLabel->obj(), lv_color_hex(0x7f8c8d), LV_PART_MAIN);
        }
    });
    
    // === DEMO 2: Real-Time Clock (1000ms) ===
    LVPanel* clockPanel = new LVPanel(container);
    clockPanel->setSize(480, 120);
    lv_obj_set_style_bg_color(clockPanel->obj(), lv_color_hex(0x2c3e50), LV_PART_MAIN);
    lv_obj_set_style_radius(clockPanel->obj(), 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(clockPanel->obj(), 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(clockPanel->obj(), lv_color_hex(0x2ecc71), LV_PART_MAIN);
    clockPanel->setPos(524, 70);
    
    LVLabel* clockTitle = new LVLabel(clockPanel);
    clockTitle->setText("[2] Clock Timer (1000ms)");
    lv_obj_set_style_text_color(clockTitle->obj(), lv_color_hex(0x2ecc71), LV_PART_MAIN);
    clockTitle->setAlign(LV_ALIGN_TOP_LEFT);
    clockTitle->setPos(15, 10);
    
    clockLabel = new LVLabel(clockPanel);
    clockLabel->setText("00:00:00");
    lv_obj_set_style_text_font(clockLabel->obj(), &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(clockLabel->obj(), lv_color_hex(0x2ecc71), LV_PART_MAIN);
    clockLabel->setAlign(LV_ALIGN_CENTER);
    clockLabel->setPos(0, 10);
    
    // Clock timer: update every second
    clockTimer = std::make_unique<LVTimer>(1000, []() {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
        clockLabel->setText(timeStr);
    });
    
    // === DEMO 3: Countdown Timer (1000ms, 10 repeats) ===
    LVPanel* countdownPanel = new LVPanel(container);
    countdownPanel->setSize(480, 180);
    lv_obj_set_style_bg_color(countdownPanel->obj(), lv_color_hex(0x2c3e50), LV_PART_MAIN);
    lv_obj_set_style_radius(countdownPanel->obj(), 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(countdownPanel->obj(), 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(countdownPanel->obj(), lv_color_hex(0xe74c3c), LV_PART_MAIN);
    countdownPanel->setPos(20, 210);
    
    LVLabel* countdownTitle = new LVLabel(countdownPanel);
    countdownTitle->setText("[3] Countdown Timer (10s)");
    lv_obj_set_style_text_color(countdownTitle->obj(), lv_color_hex(0xe74c3c), LV_PART_MAIN);
    countdownTitle->setAlign(LV_ALIGN_TOP_LEFT);
    countdownTitle->setPos(15, 10);
    
    countdownLabel = new LVLabel(countdownPanel);
    countdownLabel->setText("10");
    lv_obj_set_style_text_font(countdownLabel->obj(), &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(countdownLabel->obj(), lv_color_hex(0xe74c3c), LV_PART_MAIN);
    countdownLabel->setAlign(LV_ALIGN_CENTER);
    countdownLabel->setPos(0, 0);
    
    statusLabel = new LVLabel(countdownPanel);
    statusLabel->setText("Running...");
    lv_obj_set_style_text_color(statusLabel->obj(), lv_color_hex(0x95a5a6), LV_PART_MAIN);
    statusLabel->setAlign(LV_ALIGN_BOTTOM_MID);
    statusLabel->setPos(0, -10);
    
    // Reset button
    LVButton* resetBtn = new LVButton(countdownPanel);
    resetBtn->setText("Reset");
    resetBtn->setSize(120, 40);
    lv_obj_set_style_bg_color(resetBtn->obj(), lv_color_hex(0xe74c3c), LV_PART_MAIN);
    resetBtn->setAlign(LV_ALIGN_BOTTOM_LEFT);
    resetBtn->setPos(15, -10);
    
    lv_obj_add_event_cb(resetBtn->obj(), [](lv_event_t* e) {
        countdownValue = 10;
        countdownLabel->setText("10");
        statusLabel->setText("Reset!");
        lv_obj_set_style_text_color(countdownLabel->obj(), lv_color_hex(0xe74c3c), LV_PART_MAIN);
        if (countdownTimer) {
            countdownTimer->reset();
            countdownTimer->resume();
        }
        ESP_LOGI(TAG, "Countdown reset");
    }, LV_EVENT_CLICKED, nullptr);
    
    // Countdown timer: 1 second interval
    countdownTimer = std::make_unique<LVTimer>(1000, []() {
        countdownValue--;
        
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", countdownValue);
        countdownLabel->setText(buf);
        
        // Color coding
        if (countdownValue <= 3) {
            lv_obj_set_style_text_color(countdownLabel->obj(), lv_color_hex(0xe74c3c), LV_PART_MAIN);
            statusLabel->setText(LV_SYMBOL_WARNING " Almost done!");
        } else if (countdownValue <= 5) {
            lv_obj_set_style_text_color(countdownLabel->obj(), lv_color_hex(0xf39c12), LV_PART_MAIN);
            statusLabel->setText(LV_SYMBOL_REFRESH " Halfway!");
        }
        
        if (countdownValue <= 0) {
            countdownTimer->pause();
            countdownLabel->setText("DONE!");
            statusLabel->setText(LV_SYMBOL_OK " Finished!");
            lv_obj_set_style_text_color(countdownLabel->obj(), lv_color_hex(0x2ecc71), LV_PART_MAIN);
            ESP_LOGI(TAG, "Countdown finished!");
        }
    });
    
    // === DEMO 4: Progress Arc Animation (50ms) ===
    LVPanel* arcPanel = new LVPanel(container);
    arcPanel->setSize(480, 180);
    lv_obj_set_style_bg_color(arcPanel->obj(), lv_color_hex(0x2c3e50), LV_PART_MAIN);
    lv_obj_set_style_radius(arcPanel->obj(), 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(arcPanel->obj(), 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(arcPanel->obj(), lv_color_hex(0x9b59b6), LV_PART_MAIN);
    arcPanel->setPos(524, 210);
    
    LVLabel* arcTitle = new LVLabel(arcPanel);
    arcTitle->setText("[4] Progress Animation (50ms)");
    lv_obj_set_style_text_color(arcTitle->obj(), lv_color_hex(0x9b59b6), LV_PART_MAIN);
    arcTitle->setAlign(LV_ALIGN_TOP_LEFT);
    arcTitle->setPos(15, 10);
    
    progressArc = new LVArc(arcPanel);
    progressArc->setSize(140, 140);
    progressArc->setRange(0, 100);
    progressArc->setValue(0);
    lv_obj_set_style_arc_color(progressArc->obj(), lv_color_hex(0x9b59b6), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(progressArc->obj(), 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(progressArc->obj(), 12, LV_PART_MAIN);
    progressArc->setAlign(LV_ALIGN_CENTER);
    progressArc->setPos(0, 15);
    
    // Arc animation timer: fast update (50ms)
    arcTimer = std::make_unique<LVTimer>(50, []() {
        arcValue = (arcValue + 2) % 101;
        progressArc->setValue(arcValue);
    });
    
    // Info panel
    LVPanel* infoPanel = new LVPanel(container);
    infoPanel->setSize(984, 150);
    lv_obj_set_style_bg_color(infoPanel->obj(), lv_color_hex(0x34495e), LV_PART_MAIN);
    lv_obj_set_style_radius(infoPanel->obj(), 10, LV_PART_MAIN);
    infoPanel->setPos(20, 410);
    
    LVLabel* infoLabel = new LVLabel(infoPanel);
    infoLabel->setText(
        LV_SYMBOL_LIST " Timer Statistics:\n"
        LV_SYMBOL_RIGHT " Blink: 500ms period (toggles LED indicator)\n"
        LV_SYMBOL_RIGHT " Clock: 1000ms period (updates real-time clock)\n"
        LV_SYMBOL_RIGHT " Countdown: 1000ms period (counts down from 10 to 0)\n"
        LV_SYMBOL_RIGHT " Progress: 50ms period (smooth arc animation 0-100%)"
    );
    lv_obj_set_style_text_color(infoLabel->obj(), lv_color_hex(0xecf0f1), LV_PART_MAIN);
    infoLabel->setAlign(LV_ALIGN_TOP_LEFT);
    infoLabel->setPos(20, 15);
    
    ESP_LOGI(TAG, "✅ Timer Demo created");
    ESP_LOGI(TAG, "   - 4 timers running with different periods");
    ESP_LOGI(TAG, "   - Demonstrating: blink, clock, countdown, animation");
}

extern "C" void cleanup_timer_demo()
{
    ESP_LOGI(TAG, "Cleaning up timer demo...");
    
    // Smart pointers will automatically delete timers when reset
    blinkTimer.reset();
    clockTimer.reset();
    countdownTimer.reset();
    arcTimer.reset();
    
    // Clear widget pointers
    blinkLabel = nullptr;
    clockLabel = nullptr;
    countdownLabel = nullptr;
    statusLabel = nullptr;
    progressArc = nullptr;
    
    ESP_LOGI(TAG, "Timer demo cleanup complete");
}
