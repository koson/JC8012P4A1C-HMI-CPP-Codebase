/**
 * @file window_demo.cpp
 * @brief Demo for LVWindow - Window widget testing
 * 
 * Tests:
 * - Window with header and content
 * - Multiple windows
 * - Close button
 * - Draggable windows
 */

#include "../include/LVPanel.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVTextArea.hpp"
#include "../include/LVSlider.hpp"
#include "../include/screenshot.hpp"

class WindowDemoScreen : public LVPanel {
public:
    WindowDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        createUI();
    }

private:
    void createUI() {
        // Title
        auto* title = new LVLabel(this);
        title->setText("Window Demo - Click buttons to open windows");
        title->setTextColor(lv_color_hex(0xECF0F1));
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_20, 0);
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 10);
        
        // Button panel
        auto* btnPanel = new LVPanel(this);
        btnPanel->setSize(700, 80);
        btnPanel->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(btnPanel->obj(), 50);
        lv_obj_set_style_bg_color(btnPanel->obj(), lv_color_hex(0x34495E), LV_PART_MAIN);
        lv_obj_set_flex_flow(btnPanel->obj(), LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btnPanel->obj(), LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Create window buttons
        auto* btn1 = new LVButton(btnPanel);
        btn1->setSize(150, 50);
        btn1->setText("Info");
        lv_obj_set_style_bg_color(btn1->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
        btn1->onClicked([this](LVWidget* w, lv_event_t* e) {
            createInfoWindow();
        });
        
        auto* btn2 = new LVButton(btnPanel);
        btn2->setSize(150, 50);
        btn2->setText("Settings");
        lv_obj_set_style_bg_color(btn2->obj(), lv_color_hex(0x9B59B6), LV_PART_MAIN);
        btn2->onClicked([this](LVWidget* w, lv_event_t* e) {
            createSettingsWindow();
        });
        
        auto* btn3 = new LVButton(btnPanel);
        btn3->setSize(150, 50);
        btn3->setText("Editor");
        lv_obj_set_style_bg_color(btn3->obj(), lv_color_hex(0x2ECC71), LV_PART_MAIN);
        btn3->onClicked([this](LVWidget* w, lv_event_t* e) {
            createEditorWindow();
        });
        
        auto* btn4 = new LVButton(btnPanel);
        btn4->setSize(150, 50);
        btn4->setText("About");
        lv_obj_set_style_bg_color(btn4->obj(), lv_color_hex(0xE67E22), LV_PART_MAIN);
        btn4->onClicked([this](LVWidget* w, lv_event_t* e) {
            createAboutWindow();
        });
        
        // Screenshot button
        // auto* screenshotBtn = new LVButton(this);
        // screenshotBtn->setSize(200, 60);
        // screenshotBtn->setText(LV_SYMBOL_IMAGE " Screenshot");
        // lv_obj_set_style_bg_color(screenshotBtn->obj(), lv_color_hex(0x27AE60), LV_PART_MAIN);
        // lv_obj_align(screenshotBtn->obj(), LV_ALIGN_BOTTOM_MID, 0, -20);
        // screenshotBtn->onClicked([](LVWidget* w, lv_event_t* e) {
        //     Screenshot::capture();
        // });
    }
    
    void createInfoWindow() {
        // Create window
        auto* win = new LVPanel(this);
        win->setSize(400, 300);
        lv_obj_align(win->obj(), LV_ALIGN_CENTER, -150, 0);
        lv_obj_set_style_bg_color(win->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(win->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(win->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_shadow_width(win->obj(), 15, LV_PART_MAIN);
        lv_obj_add_flag(win->obj(), LV_OBJ_FLAG_FLOATING);
        
        // Header
        auto* header = new LVPanel(win);
        header->setSize(LV_PCT(100), 40);
        lv_obj_set_style_bg_color(header->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_radius(header->obj(), 0, LV_PART_MAIN);
        
        auto* titleLabel = new LVLabel(header);
        titleLabel->setText(LV_SYMBOL_HOME " Information");
        titleLabel->setTextColor(lv_color_hex(0xFFFFFF));
        lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_18, 0);
        lv_obj_set_pos(titleLabel->obj(), 10, 8);
        
        auto* closeBtn = new LVButton(header);
        closeBtn->setSize(30, 30);
        closeBtn->setText(LV_SYMBOL_CLOSE);
        lv_obj_align(closeBtn->obj(), LV_ALIGN_RIGHT_MID, -5, 0);
        lv_obj_set_style_bg_color(closeBtn->obj(), lv_color_hex(0xE74C3C), LV_PART_MAIN);
        closeBtn->onClicked([win](LVWidget* w, lv_event_t* e) {
            lv_obj_delete(win->obj());
        });
        
        // Content
        auto* content = new LVLabel(win);
        content->setText(
            "System Information\n\n"
            "Device: ESP32-P4\n"
            "CPU: Dual-core RISC-V\n"
            "RAM: 512KB SRAM\n"
            "Display: 1024x600\n"
            "LVGL: v9.2.2\n\n"
            "This is a draggable window\n"
            "with custom styling."
        );
        content->setTextColor(lv_color_hex(0x2C3E50));
        lv_obj_set_pos(content->obj(), 20, 60);
        lv_obj_set_style_text_line_space(content->obj(), 8, 0);
    }
    
    void createSettingsWindow() {
        auto* win = new LVPanel(this);
        win->setSize(450, 350);
        lv_obj_align(win->obj(), LV_ALIGN_CENTER, 0, -50);
        lv_obj_set_style_bg_color(win->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(win->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(win->obj(), lv_color_hex(0x9B59B6), LV_PART_MAIN);
        lv_obj_set_style_shadow_width(win->obj(), 15, LV_PART_MAIN);
        lv_obj_add_flag(win->obj(), LV_OBJ_FLAG_FLOATING);
        
        // Header
        auto* header = new LVPanel(win);
        header->setSize(LV_PCT(100), 40);
        lv_obj_set_style_bg_color(header->obj(), lv_color_hex(0x9B59B6), LV_PART_MAIN);
        lv_obj_set_style_radius(header->obj(), 0, LV_PART_MAIN);
        
        auto* titleLabel = new LVLabel(header);
        titleLabel->setText(LV_SYMBOL_SETTINGS " Settings");
        titleLabel->setTextColor(lv_color_hex(0xFFFFFF));
        lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_18, 0);
        lv_obj_set_pos(titleLabel->obj(), 10, 8);
        
        auto* closeBtn = new LVButton(header);
        closeBtn->setSize(30, 30);
        closeBtn->setText(LV_SYMBOL_CLOSE);
        lv_obj_align(closeBtn->obj(), LV_ALIGN_RIGHT_MID, -5, 0);
        lv_obj_set_style_bg_color(closeBtn->obj(), lv_color_hex(0xE74C3C), LV_PART_MAIN);
        closeBtn->onClicked([win](LVWidget* w, lv_event_t* e) {
            lv_obj_delete(win->obj());
        });
        
        // Content - Sliders
        auto* volumeLabel = new LVLabel(win);
        volumeLabel->setText("Volume");
        volumeLabel->setTextColor(lv_color_hex(0x2C3E50));
        lv_obj_set_pos(volumeLabel->obj(), 20, 60);
        
        auto* volumeSlider = new LVSlider(win);
        volumeSlider->setSize(400, 20);
        lv_obj_set_pos(volumeSlider->obj(), 20, 90);
        volumeSlider->setValue(75);
        
        auto* brightnessLabel = new LVLabel(win);
        brightnessLabel->setText("Brightness");
        brightnessLabel->setTextColor(lv_color_hex(0x2C3E50));
        lv_obj_set_pos(brightnessLabel->obj(), 20, 130);
        
        auto* brightnessSlider = new LVSlider(win);
        brightnessSlider->setSize(400, 20);
        lv_obj_set_pos(brightnessSlider->obj(), 20, 160);
        brightnessSlider->setValue(85);
        
        auto* contrastLabel = new LVLabel(win);
        contrastLabel->setText("Contrast");
        contrastLabel->setTextColor(lv_color_hex(0x2C3E50));
        lv_obj_set_pos(contrastLabel->obj(), 20, 200);
        
        auto* contrastSlider = new LVSlider(win);
        contrastSlider->setSize(400, 20);
        lv_obj_set_pos(contrastSlider->obj(), 20, 230);
        contrastSlider->setValue(50);
        
        // Apply button
        auto* applyBtn = new LVButton(win);
        applyBtn->setSize(120, 40);
        applyBtn->setText("Apply");
        lv_obj_set_pos(applyBtn->obj(), 160, 280);
        lv_obj_set_style_bg_color(applyBtn->obj(), lv_color_hex(0x2ECC71), LV_PART_MAIN);
    }
    
    void createEditorWindow() {
        auto* win = new LVPanel(this);
        win->setSize(500, 400);
        lv_obj_align(win->obj(), LV_ALIGN_CENTER, 100, 0);
        lv_obj_set_style_bg_color(win->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(win->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(win->obj(), lv_color_hex(0x2ECC71), LV_PART_MAIN);
        lv_obj_set_style_shadow_width(win->obj(), 15, LV_PART_MAIN);
        lv_obj_add_flag(win->obj(), LV_OBJ_FLAG_FLOATING);
        
        // Header
        auto* header = new LVPanel(win);
        header->setSize(LV_PCT(100), 40);
        lv_obj_set_style_bg_color(header->obj(), lv_color_hex(0x2ECC71), LV_PART_MAIN);
        lv_obj_set_style_radius(header->obj(), 0, LV_PART_MAIN);
        
        auto* titleLabel = new LVLabel(header);
        titleLabel->setText(LV_SYMBOL_EDIT " Text Editor");
        titleLabel->setTextColor(lv_color_hex(0xFFFFFF));
        lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_18, 0);
        lv_obj_set_pos(titleLabel->obj(), 10, 8);
        
        auto* closeBtn = new LVButton(header);
        closeBtn->setSize(30, 30);
        closeBtn->setText(LV_SYMBOL_CLOSE);
        lv_obj_align(closeBtn->obj(), LV_ALIGN_RIGHT_MID, -5, 0);
        lv_obj_set_style_bg_color(closeBtn->obj(), lv_color_hex(0xE74C3C), LV_PART_MAIN);
        closeBtn->onClicked([win](LVWidget* w, lv_event_t* e) {
            lv_obj_delete(win->obj());
        });
        
        // TextArea
        auto* textArea = new LVTextArea(win);
        textArea->setSize(460, 300);
        lv_obj_set_pos(textArea->obj(), 20, 60);
        textArea->setText("Type your text here...\n\nThis is a multi-line text editor\ninside a window.");
    }
    
    void createAboutWindow() {
        auto* win = new LVPanel(this);
        win->setSize(380, 280);
        lv_obj_align(win->obj(), LV_ALIGN_CENTER, 0, 50);
        lv_obj_set_style_bg_color(win->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(win->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(win->obj(), lv_color_hex(0xE67E22), LV_PART_MAIN);
        lv_obj_set_style_shadow_width(win->obj(), 15, LV_PART_MAIN);
        lv_obj_add_flag(win->obj(), LV_OBJ_FLAG_FLOATING);
        
        // Header
        auto* header = new LVPanel(win);
        header->setSize(LV_PCT(100), 40);
        lv_obj_set_style_bg_color(header->obj(), lv_color_hex(0xE67E22), LV_PART_MAIN);
        lv_obj_set_style_radius(header->obj(), 0, LV_PART_MAIN);
        
        auto* titleLabel = new LVLabel(header);
        titleLabel->setText(LV_SYMBOL_LIST " About");
        titleLabel->setTextColor(lv_color_hex(0xFFFFFF));
        lv_obj_set_style_text_font(titleLabel->obj(), &lv_font_montserrat_18, 0);
        lv_obj_set_pos(titleLabel->obj(), 10, 8);
        
        auto* closeBtn = new LVButton(header);
        closeBtn->setSize(30, 30);
        closeBtn->setText(LV_SYMBOL_CLOSE);
        lv_obj_align(closeBtn->obj(), LV_ALIGN_RIGHT_MID, -5, 0);
        lv_obj_set_style_bg_color(closeBtn->obj(), lv_color_hex(0xE74C3C), LV_PART_MAIN);
        closeBtn->onClicked([win](LVWidget* w, lv_event_t* e) {
            lv_obj_delete(win->obj());
        });
        
        // Content
        auto* content = new LVLabel(win);
        content->setText(
            "LVGL Window Demo\n\n"
            "Version: 1.0.0\n"
            "Build: 2026.01.15\n\n"
            "Features:\n"
            "• Multiple windows\n"
            "• Custom styling\n"
            "• Close buttons\n"
            "• Floating windows"
        );
        content->setTextColor(lv_color_hex(0x2C3E50));
        lv_obj_set_pos(content->obj(), 20, 60);
        lv_obj_set_style_text_line_space(content->obj(), 6, 0);
    }
};

extern "C" void create_window_demo() {
    auto* screen = new WindowDemoScreen();
    lv_obj_set_parent(screen->obj(), lv_screen_active());
}
