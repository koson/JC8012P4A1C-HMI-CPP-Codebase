/**
 * @file textarea_editor.cpp
 * @brief Phase 2 Test #8 - LVTextArea (Note Editor)
 * 
 * Tests:
 * - TC1: Create multi-line text area
 * - TC2: Placeholder text visibility
 * - TC3: Text input and display
 * - TC4: One-line mode vs multi-line mode
 * - TC5: Max length constraint
 * - TC6: Password mode
 * - TC7: Clear text functionality
 * - TC8: Character counter update
 */

#include "../include/LVTextArea.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVPanel.hpp"
#include "esp_log.h"
#include <string>

static const char* TAG = "TEXTAREA_TEST";

// Global widgets
static LVTextArea* txtNote = nullptr;
static LVTextArea* txtPassword = nullptr;
static LVLabel* lblCharCount = nullptr;
static LVLabel* lblStatus = nullptr;

/**
 * @brief Update character counter
 */
static void updateCharCounter()
{
    if (!txtNote || !lblCharCount) return;
    
    std::string text = txtNote->getText();
    uint32_t len = text.length();
    
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Characters: %lu / 200", len);
    lblCharCount->setText(buffer);
    
    // Color coding based on length
    if (len >= 180) {
        lblCharCount->setTextColor(lv_color_hex(0xD32F2F)); // Red
    } else if (len >= 150) {
        lblCharCount->setTextColor(lv_color_hex(0xF57C00)); // Orange
    } else {
        lblCharCount->setTextColor(lv_color_hex(0x1976D2)); // Blue
    }
    
    ESP_LOGI(TAG, "Character count: %lu", len);
}

/**
 * @brief Text area value changed callback
 */
static void onTextAreaChanged(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_VALUE_CHANGED) return;
    
    lv_obj_t* ta = (lv_obj_t*)lv_event_get_target(e);
    const char* text = lv_textarea_get_text(ta);
    
    ESP_LOGI(TAG, "Text changed: '%s'", text);
    updateCharCounter();
}

/**
 * @brief Clear button clicked callback
 */
static void onClearClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    if (txtNote) {
        txtNote->clearText();
        lblStatus->setText("🗑️ Note cleared");
        lblStatus->setTextColor(lv_color_hex(0x388E3C));
        ESP_LOGI(TAG, "Note cleared");
    }
}

/**
 * @brief Save button clicked callback
 */
static void onSaveClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    if (txtNote) {
        std::string text = txtNote->getText();
        if (text.empty()) {
            lblStatus->setText("⚠️ Note is empty");
            lblStatus->setTextColor(lv_color_hex(0xF57C00));
            ESP_LOGW(TAG, "Cannot save empty note");
        } else {
            lblStatus->setText("💾 Note saved successfully");
            lblStatus->setTextColor(lv_color_hex(0x1976D2));
            ESP_LOGI(TAG, "Note saved: %lu characters", text.length());
        }
    }
}

/**
 * @brief Show password button clicked callback
 */
static void onShowPasswordClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    
    if (txtPassword) {
        bool isPassword = lv_textarea_get_password_mode(txtPassword->obj());
        lv_textarea_set_password_mode(txtPassword->obj(), !isPassword);
        
        lv_obj_t* label = lv_obj_get_child(btn, 0);
        if (label) {
            lv_label_set_text(label, !isPassword ? "👁️ Show" : "🔒 Hide");
        }
        
        ESP_LOGI(TAG, "Password mode: %s", !isPassword ? "ON" : "OFF");
    }
}

/**
 * @brief Create Note Editor UI
 */
extern "C" void test_textarea_editor()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #8: LVTextArea - Note Editor");
    ESP_LOGI(TAG, "========================================");
    
    // สร้าง Main container
    auto* container = new LVPanel(nullptr);
    lv_obj_set_size(container->obj(), 900, 560);
    lv_obj_align(container->obj(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(container->obj(), lv_color_hex(0xF5F5F5), 0);
    lv_obj_set_style_border_width(container->obj(), 0, 0);
    lv_obj_set_style_pad_all(container->obj(), 20, 0);
    
    // Title
    auto* lblTitle = new LVLabel(container);
    lblTitle->setText("Note Editor");
    lv_obj_set_style_text_font(lblTitle->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lblTitle->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblTitle->obj(), LV_ALIGN_TOP_MID, 0, 0);
    
    // Multi-line note section (200 chars max)
    auto* lblNoteTitle = new LVLabel(container);
    lblNoteTitle->setText("Multi-line Note (200 chars max):");
    lv_obj_set_style_text_font(lblNoteTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblNoteTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblNoteTitle->obj(), LV_ALIGN_TOP_LEFT, 0, 45);
    
    txtNote = new LVTextArea(container);
    lv_obj_set_size(txtNote->obj(), 860, 180);
    lv_obj_align(txtNote->obj(), LV_ALIGN_TOP_LEFT, 0, 75);
    txtNote->setPlaceholder("Type your note here...\nMulti-line input supported");
    txtNote->setMaxLength(200);
    lv_obj_set_style_bg_color(txtNote->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(txtNote->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_border_width(txtNote->obj(), 2, 0);
    lv_obj_set_style_text_font(txtNote->obj(), &lv_font_montserrat_18, 0);
    lv_obj_add_event_cb(txtNote->obj(), onTextAreaChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    // Character counter
    lblCharCount = new LVLabel(container);
    lblCharCount->setText("Characters: 0 / 200");
    lv_obj_set_style_text_font(lblCharCount->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblCharCount->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblCharCount->obj(), LV_ALIGN_TOP_RIGHT, 0, 260);
    
    // Password section (one-line mode)
    auto* lblPasswordTitle = new LVLabel(container);
    lblPasswordTitle->setText("Password Input (one-line, 20 chars max):");
    lv_obj_set_style_text_font(lblPasswordTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblPasswordTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblPasswordTitle->obj(), LV_ALIGN_TOP_LEFT, 0, 290);
    
    txtPassword = new LVTextArea(container);
    lv_obj_set_size(txtPassword->obj(), 680, 50);
    lv_obj_align(txtPassword->obj(), LV_ALIGN_TOP_LEFT, 0, 320);
    txtPassword->setPlaceholder("Enter password...");
    txtPassword->setMaxLength(20);
    lv_textarea_set_one_line(txtPassword->obj(), true);
    lv_textarea_set_password_mode(txtPassword->obj(), true);
    lv_obj_set_style_bg_color(txtPassword->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(txtPassword->obj(), lv_color_hex(0xD32F2F), 0);
    lv_obj_set_style_border_width(txtPassword->obj(), 2, 0);
    lv_obj_set_style_text_font(txtPassword->obj(), &lv_font_montserrat_18, 0);
    
    // Show/Hide password button
    auto* btnShowPassword = new LVButton(container);
    lv_obj_set_size(btnShowPassword->obj(), 160, 50);
    lv_obj_align(btnShowPassword->obj(), LV_ALIGN_TOP_LEFT, 700, 320);
    lv_obj_set_style_bg_color(btnShowPassword->obj(), lv_color_hex(0x757575), 0);
    btnShowPassword->setText("Show");
    btnShowPassword->setTextColor(lv_color_hex(0xFFFFFF));
    btnShowPassword->setFont(&lv_font_montserrat_18);
    
    lv_obj_add_event_cb(btnShowPassword->obj(), onShowPasswordClicked, LV_EVENT_CLICKED, nullptr);
    
    // Button panel
    auto* panelButtons = new LVPanel(container);
    lv_obj_set_size(panelButtons->obj(), 860, 60);
    lv_obj_align(panelButtons->obj(), LV_ALIGN_TOP_LEFT, 0, 390);
    lv_obj_set_style_bg_color(panelButtons->obj(), lv_color_hex(0xF5F5F5), 0);
    lv_obj_set_style_border_width(panelButtons->obj(), 0, 0);
    lv_obj_set_flex_flow(panelButtons->obj(), LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(panelButtons->obj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(panelButtons->obj(), 20, 0);
    
    // Clear button
    auto* btnClear = new LVButton(panelButtons);
    lv_obj_set_size(btnClear->obj(), 200, 50);
    lv_obj_set_style_bg_color(btnClear->obj(), lv_color_hex(0xD32F2F), 0);
    btnClear->setText("Clear");
    btnClear->setTextColor(lv_color_hex(0xFFFFFF));
    btnClear->setFont(&lv_font_montserrat_18);
    
    lv_obj_add_event_cb(btnClear->obj(), onClearClicked, LV_EVENT_CLICKED, nullptr);
    
    // Save button
    auto* btnSave = new LVButton(panelButtons);
    lv_obj_set_size(btnSave->obj(), 200, 50);
    lv_obj_set_style_bg_color(btnSave->obj(), lv_color_hex(0x388E3C), 0);
    btnSave->setText("Save");
    btnSave->setTextColor(lv_color_hex(0xFFFFFF));
    btnSave->setFont(&lv_font_montserrat_18);
    
    lv_obj_add_event_cb(btnSave->obj(), onSaveClicked, LV_EVENT_CLICKED, nullptr);
    
    // Status label
    lblStatus = new LVLabel(container);
    lblStatus->setText("Ready to write...");
    lv_obj_set_style_text_font(lblStatus->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblStatus->obj(), lv_color_hex(0x757575), 0);
    lv_obj_align(lblStatus->obj(), LV_ALIGN_BOTTOM_MID, 0, 0);
    
    ESP_LOGI(TAG, "✅ Note Editor UI created");
    ESP_LOGI(TAG, "   - Multi-line TextArea: 200 chars max");
    ESP_LOGI(TAG, "   - Password TextArea: one-line, 20 chars max");
    ESP_LOGI(TAG, "   - Character counter with color coding");
    ESP_LOGI(TAG, "   - Clear and Save buttons");
}
