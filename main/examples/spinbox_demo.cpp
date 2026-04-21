/**
 * @file spinbox_demo.cpp
 * @brief Demo for LVSpinbox - Numeric input widget testing
 * 
 * Tests:
 * - Spinbox with increment/decrement buttons
 * - Range limits
 * - Step values
 * - Digit formatting
 * - Rollover behavior
 */

#include "../include/LVPanel.hpp"
#include "../include/LVSpinbox.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"

class SpinboxDemoScreen : public LVPanel {
public:
    SpinboxDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0xF0F0F0), LV_PART_MAIN);
        createUI();
    }

private:
    LVSpinbox* spinbox1 = nullptr;
    LVSpinbox* spinbox2 = nullptr;
    LVSpinbox* spinbox3 = nullptr;
    LVLabel* valueLabel1 = nullptr;
    LVLabel* valueLabel2 = nullptr;
    LVLabel* valueLabel3 = nullptr;
    
    void createUI() {
        // Title
        auto* title = new LVLabel(this);
        title->setText("Spinbox Demo");
        title->setTextColor(lv_color_hex(0x333333));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 10);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_24, 0);
        
        // ===== Spinbox 1: Basic Integer (0-100) =====
        auto* label1Title = new LVLabel(this);
        label1Title->setText("Integer (0-100)");
        label1Title->setTextColor(lv_color_hex(0x333333));
        lv_obj_set_pos(label1Title->obj(), 50, 80);
        lv_obj_set_style_text_font(label1Title->obj(), &lv_font_montserrat_18, 0);
        
        spinbox1 = new LVSpinbox(this);
        lv_obj_set_pos(spinbox1->obj(), 50, 110);
        lv_obj_set_size(spinbox1->obj(), 200, 50);
        spinbox1->setRange(0, 100);
        spinbox1->setStep(1);
        spinbox1->setValue(50);
        spinbox1->setDigitFormat(3, 0);  // 3 digits, no separator
        
        // +/- buttons for spinbox1
        auto* btnInc1 = new LVButton(this);
        lv_obj_set_pos(btnInc1->obj(), 260, 110);
        lv_obj_set_size(btnInc1->obj(), 50, 50);
        btnInc1->setText("+");
        lv_obj_set_style_text_font(btnInc1->obj(), &lv_font_montserrat_24, 0);
        btnInc1->onClicked([this](LVWidget* w, lv_event_t* e) {
            spinbox1->increment();
        });
        
        auto* btnDec1 = new LVButton(this);
        lv_obj_set_pos(btnDec1->obj(), 320, 110);
        lv_obj_set_size(btnDec1->obj(), 50, 50);
        btnDec1->setText("-");
        lv_obj_set_style_text_font(btnDec1->obj(), &lv_font_montserrat_24, 0);
        btnDec1->onClicked([this](LVWidget* w, lv_event_t* e) {
            spinbox1->decrement();
        });
        
        valueLabel1 = new LVLabel(this);
        valueLabel1->setText("Value: 50");
        valueLabel1->setTextColor(lv_color_hex(0x0066CC));
        lv_obj_set_pos(valueLabel1->obj(), 390, 120);
        
        spinbox1->onChange([this](int32_t value) {
            char buf[32];
            sprintf(buf, "Value: %d", (int)value);
            valueLabel1->setText(buf);
        });
        
        // ===== Spinbox 2: Decimal (0.00-10.00) =====
        auto* label2Title = new LVLabel(this);
        label2Title->setText("Decimal (0.00-10.00)");
        label2Title->setTextColor(lv_color_hex(0x333333));
        lv_obj_set_pos(label2Title->obj(), 50, 190);
        lv_obj_set_style_text_font(label2Title->obj(), &lv_font_montserrat_18, 0);
        
        spinbox2 = new LVSpinbox(this);
        lv_obj_set_pos(spinbox2->obj(), 50, 220);
        lv_obj_set_size(spinbox2->obj(), 200, 50);
        spinbox2->setRange(0, 1000);  // 0-10.00 represented as 0-1000
        spinbox2->setStep(5);  // Step by 0.05
        spinbox2->setValue(500);  // 5.00
        spinbox2->setDigitFormat(4, 2);  // 4 digits, decimal after 2nd position
        
        auto* btnInc2 = new LVButton(this);
        lv_obj_set_pos(btnInc2->obj(), 260, 220);
        lv_obj_set_size(btnInc2->obj(), 50, 50);
        btnInc2->setText("+");
        lv_obj_set_style_text_font(btnInc2->obj(), &lv_font_montserrat_24, 0);
        btnInc2->onClicked([this](LVWidget* w, lv_event_t* e) {
            spinbox2->increment();
        });
        
        auto* btnDec2 = new LVButton(this);
        lv_obj_set_pos(btnDec2->obj(), 320, 220);
        lv_obj_set_size(btnDec2->obj(), 50, 50);
        btnDec2->setText("-");
        lv_obj_set_style_text_font(btnDec2->obj(), &lv_font_montserrat_24, 0);
        btnDec2->onClicked([this](LVWidget* w, lv_event_t* e) {
            spinbox2->decrement();
        });
        
        valueLabel2 = new LVLabel(this);
        valueLabel2->setText("Value: 5.00");
        valueLabel2->setTextColor(lv_color_hex(0x0066CC));
        lv_obj_set_pos(valueLabel2->obj(), 390, 230);
        
        spinbox2->onChange([this](int32_t value) {
            char buf[32];
            sprintf(buf, "Value: %.2f", value / 100.0f);
            valueLabel2->setText(buf);
        });
        
        // ===== Spinbox 3: Large Step with Rollover =====
        auto* label3Title = new LVLabel(this);
        label3Title->setText("Step 10, Rollover (0-100)");
        label3Title->setTextColor(lv_color_hex(0x333333));
        lv_obj_set_pos(label3Title->obj(), 50, 300);
        lv_obj_set_style_text_font(label3Title->obj(), &lv_font_montserrat_18, 0);
        
        spinbox3 = new LVSpinbox(this);
        lv_obj_set_pos(spinbox3->obj(), 50, 330);
        lv_obj_set_size(spinbox3->obj(), 200, 50);
        spinbox3->setRange(0, 100);
        spinbox3->setStep(10);  // Jump by 10
        spinbox3->setValue(0);
        spinbox3->setDigitFormat(3, 0);
        spinbox3->setRollover(true);  // Loop from max to min
        
        auto* btnInc3 = new LVButton(this);
        lv_obj_set_pos(btnInc3->obj(), 260, 330);
        lv_obj_set_size(btnInc3->obj(), 50, 50);
        btnInc3->setText("+");
        lv_obj_set_style_text_font(btnInc3->obj(), &lv_font_montserrat_24, 0);
        btnInc3->onClicked([this](LVWidget* w, lv_event_t* e) {
            spinbox3->increment();
        });
        
        auto* btnDec3 = new LVButton(this);
        lv_obj_set_pos(btnDec3->obj(), 320, 330);
        lv_obj_set_size(btnDec3->obj(), 50, 50);
        btnDec3->setText("-");
        lv_obj_set_style_text_font(btnDec3->obj(), &lv_font_montserrat_24, 0);
        btnDec3->onClicked([this](LVWidget* w, lv_event_t* e) {
            spinbox3->decrement();
        });
        
        valueLabel3 = new LVLabel(this);
        valueLabel3->setText("Value: 0 (Rollover enabled)");
        valueLabel3->setTextColor(lv_color_hex(0x0066CC));
        lv_obj_set_pos(valueLabel3->obj(), 390, 340);
        
        spinbox3->onChange([this](int32_t value) {
            char buf[48];
            sprintf(buf, "Value: %d (Rollover enabled)", (int)value);
            valueLabel3->setText(buf);
        });
        
        // Info panel
        auto* infoPanel = new LVPanel(this);
        infoPanel->setSize(400, 200);
        lv_obj_set_pos(infoPanel->obj(), 550, 80);
        lv_obj_set_style_bg_color(infoPanel->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(infoPanel->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(infoPanel->obj(), lv_color_hex(0xDDDDDD), LV_PART_MAIN);
        lv_obj_set_style_pad_all(infoPanel->obj(), 20, LV_PART_MAIN);
        
        auto* infoTitle = new LVLabel(infoPanel);
        infoTitle->setText("Spinbox Features");
        infoTitle->setTextColor(lv_color_hex(0x333333));
        lv_obj_set_pos(infoTitle->obj(), 0, 0);
        lv_obj_set_style_text_font(infoTitle->obj(), &lv_font_montserrat_18, 0);
        
        auto* infoText = new LVLabel(infoPanel);
        infoText->setText(
            "• Click +/- to adjust value\n"
            "• Range limits enforced\n"
            "• Custom step values\n"
            "• Decimal formatting\n"
            "• Rollover at limits\n\n"
            "Spinbox 3 loops from\n"
            "100 back to 0"
        );
        infoText->setTextColor(lv_color_hex(0x666666));
        lv_obj_set_pos(infoText->obj(), 0, 35);
        lv_obj_set_style_text_line_space(infoText->obj(), 8, 0);
    }
};

extern "C" void create_spinbox_demo() {
    auto* screen = new SpinboxDemoScreen();
    lv_obj_set_parent(screen->obj(), lv_screen_active());
}
