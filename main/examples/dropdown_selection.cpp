/**
 * @file dropdown_selection.cpp
 * @brief Phase 2 Testing: LVDropdown Comprehensive Testing
 * 
 * Test Cases:
 * - Dropdown creation with multiple options
 * - Option selection via touch
 * - VALUE_CHANGED event handling
 * - Multiple dropdowns on same screen
 * - Option list expand/collapse
 * - Selected value retrieval
 */

#include "lvgl.h"
#include "LVWidget.hpp"
#include "LVDropdown.hpp"
#include "LVLabel.hpp"
#include "LVPanel.hpp"
#include "esp_log.h"
#include <string>

static const char* TAG = "DROPDOWN_TEST";

// UI Components
static LVDropdown* ddTheme = nullptr;
static LVDropdown* ddLanguage = nullptr;
static LVDropdown* ddFontSize = nullptr;
static LVLabel* lblStatus = nullptr;
static LVLabel* lblCurrentTheme = nullptr;
static LVLabel* lblCurrentLanguage = nullptr;
static LVLabel* lblCurrentFontSize = nullptr;

// Configuration options
static const char* THEME_OPTIONS = "Dark Mode\nLight Mode\nAuto\nHigh Contrast";
static const char* LANGUAGE_OPTIONS = "English\nไทย (Thai)\n中文 (Chinese)\n日本語 (Japanese)\nEspañol\nFrançais";
static const char* FONTSIZE_OPTIONS = "Small (14)\nMedium (18)\nLarge (24)\nExtra Large (32)";

// Current selections
static std::string currentTheme = "Dark Mode";
static std::string currentLanguage = "English";
static std::string currentFontSize = "Medium (18)";

/**
 * Update status label with all current selections
 */
static void updateStatus()
{
    char statusText[200];
    snprintf(statusText, sizeof(statusText), 
             "📱 Settings Applied:\n• Theme: %s\n• Language: %s\n• Font: %s",
             currentTheme.c_str(),
             currentLanguage.c_str(),
             currentFontSize.c_str());
    
    lblStatus->setText(statusText);
    
    // Change status color based on theme
    ESP_LOGI(TAG, "updateStatus() - Checking theme: '%s' (len=%d)", currentTheme.c_str(), currentTheme.length());
    
    if (currentTheme == "Dark Mode") {
        ESP_LOGI(TAG, "Setting Dark Blue color");
        lblStatus->setTextColor(lv_color_hex(0x1565C0)); // Dark blue (เข้มกว่าเดิม)
    } else if (currentTheme == "Light Mode") {
        ESP_LOGI(TAG, "Setting Orange color");
        lblStatus->setTextColor(lv_color_hex(0xE65100)); // Dark orange (เข้มกว่าเดิม)
    } else if (currentTheme == "Auto") {
        ESP_LOGI(TAG, "Setting Purple color");
        lblStatus->setTextColor(lv_color_hex(0x6A1B9A)); // Dark purple (เข้มกว่าเดิม)
    } else {
        ESP_LOGI(TAG, "Setting Red color (default)");
        lblStatus->setTextColor(lv_color_hex(0xC62828)); // Dark red (เข้มกว่าเดิม)
    }
}

/**
 * Theme dropdown changed event handler
 */
static void onThemeChanged(lv_event_t* e)
{
    lv_obj_t* dropdown = (lv_obj_t*)lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
    currentTheme = buf;
    
    ESP_LOGI(TAG, "Theme changed to: %s (index: %d)", currentTheme.c_str(), selected);
    
    lblCurrentTheme->setText(currentTheme);
    updateStatus();
}

/**
 * Language dropdown changed event handler
 */
static void onLanguageChanged(lv_event_t* e)
{
    lv_obj_t* dropdown = (lv_obj_t*)lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
    currentLanguage = buf;
    
    ESP_LOGI(TAG, "Language changed to: %s (index: %d)", currentLanguage.c_str(), selected);
    
    lblCurrentLanguage->setText(currentLanguage);
    updateStatus();
}

/**
 * Font size dropdown changed event handler
 */
static void onFontSizeChanged(lv_event_t* e)
{
    lv_obj_t* dropdown = (lv_obj_t*)lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
    currentFontSize = buf;
    
    ESP_LOGI(TAG, "Font size changed to: %s (index: %d)", currentFontSize.c_str(), selected);
    
    lblCurrentFontSize->setText(currentFontSize);
    updateStatus();
}

/**
 * Main test function
 */
extern "C" void test_dropdown_selection()
{
    ESP_LOGI(TAG, "\n=== Dropdown Selection Example ===");
    ESP_LOGI(TAG, "Testing: LVDropdown with multiple options\n");
    
    // Create main container
    LVPanel* container = new LVPanel(nullptr);
    container->setSize(900, 580);
    lv_obj_align(container->obj(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(container->obj(), lv_color_hex(0xF5F5F5), 0);
    lv_obj_set_style_border_width(container->obj(), 2, 0);
    lv_obj_set_style_border_color(container->obj(), lv_color_hex(0xBDBDBD), 0);
    lv_obj_set_style_radius(container->obj(), 12, 0);
    lv_obj_set_style_pad_all(container->obj(), 20, 0);
    
    // Title
    LVLabel* lblTitle = new LVLabel(container);
    lblTitle->setText("⚙️ Application Settings");
    lblTitle->setFont(&lv_font_montserrat_24);
    lblTitle->setTextColor(lv_color_hex(0x212121));
    lv_obj_align(lblTitle->obj(), LV_ALIGN_TOP_MID, 0, 0);
    
    // ==================== Theme Selection ====================
    int yPos = 60;
    
    LVLabel* lblThemeTitle = new LVLabel(container);
    lblThemeTitle->setText("🎨 Theme:");
    lblThemeTitle->setFont(&lv_font_montserrat_18);
    lblThemeTitle->setTextColor(lv_color_hex(0x424242));
    lv_obj_align(lblThemeTitle->obj(), LV_ALIGN_TOP_LEFT, 0, yPos);
    
    ddTheme = new LVDropdown(container);
    ddTheme->setOptions(THEME_OPTIONS);
    ddTheme->setSelected(0); // Dark Mode
    ddTheme->setSize(350, 50);
    lv_obj_align(ddTheme->obj(), LV_ALIGN_TOP_LEFT, 150, yPos - 5);
    lv_obj_set_style_bg_color(ddTheme->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ddTheme->obj(), &lv_font_montserrat_16, 0);
    lv_obj_add_event_cb(ddTheme->obj(), onThemeChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    lblCurrentTheme = new LVLabel(container);
    lblCurrentTheme->setText(currentTheme);
    lblCurrentTheme->setFont(&lv_font_montserrat_16);
    lblCurrentTheme->setTextColor(lv_color_hex(0x1976D2));
    lv_obj_align(lblCurrentTheme->obj(), LV_ALIGN_TOP_LEFT, 520, yPos);
    
    // ==================== Language Selection ====================
    yPos += 80;
    
    LVLabel* lblLanguageTitle = new LVLabel(container);
    lblLanguageTitle->setText("🌐 Language:");
    lblLanguageTitle->setFont(&lv_font_montserrat_18);
    lblLanguageTitle->setTextColor(lv_color_hex(0x424242));
    lv_obj_align(lblLanguageTitle->obj(), LV_ALIGN_TOP_LEFT, 0, yPos);
    
    ddLanguage = new LVDropdown(container);
    ddLanguage->setOptions(LANGUAGE_OPTIONS);
    ddLanguage->setSelected(0); // English
    ddLanguage->setSize(350, 50);
    lv_obj_align(ddLanguage->obj(), LV_ALIGN_TOP_LEFT, 150, yPos - 5);
    lv_obj_set_style_bg_color(ddLanguage->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ddLanguage->obj(), &lv_font_montserrat_16, 0);
    lv_obj_add_event_cb(ddLanguage->obj(), onLanguageChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    // Set dropdown list height (show more options)
    lv_obj_t* list = lv_dropdown_get_list(ddLanguage->obj());
    if (list) {
        lv_obj_set_height(list, 300); // Show 6 options at once
    }
    
    lblCurrentLanguage = new LVLabel(container);
    lblCurrentLanguage->setText(currentLanguage);
    lblCurrentLanguage->setFont(&lv_font_montserrat_16);
    lblCurrentLanguage->setTextColor(lv_color_hex(0x388E3C));
    lv_obj_align(lblCurrentLanguage->obj(), LV_ALIGN_TOP_LEFT, 520, yPos);
    
    // ==================== Font Size Selection ====================
    yPos += 80;
    
    LVLabel* lblFontSizeTitle = new LVLabel(container);
    lblFontSizeTitle->setText("🔤 Font Size:");
    lblFontSizeTitle->setFont(&lv_font_montserrat_18);
    lblFontSizeTitle->setTextColor(lv_color_hex(0x424242));
    lv_obj_align(lblFontSizeTitle->obj(), LV_ALIGN_TOP_LEFT, 0, yPos);
    
    ddFontSize = new LVDropdown(container);
    ddFontSize->setOptions(FONTSIZE_OPTIONS);
    ddFontSize->setSelected(1); // Medium (18)
    ddFontSize->setSize(350, 50);
    lv_obj_align(ddFontSize->obj(), LV_ALIGN_TOP_LEFT, 150, yPos - 5);
    lv_obj_set_style_bg_color(ddFontSize->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ddFontSize->obj(), &lv_font_montserrat_16, 0);
    lv_obj_add_event_cb(ddFontSize->obj(), onFontSizeChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    
    lblCurrentFontSize = new LVLabel(container);
    lblCurrentFontSize->setText(currentFontSize);
    lblCurrentFontSize->setFont(&lv_font_montserrat_16);
    lblCurrentFontSize->setTextColor(lv_color_hex(0xF57C00));
    lv_obj_align(lblCurrentFontSize->obj(), LV_ALIGN_TOP_LEFT, 520, yPos);
    
    // ==================== Status Display ====================
    yPos += 100;
    
    // Status panel
    LVPanel* statusPanel = new LVPanel(container);
    statusPanel->setSize(860, 180);
    lv_obj_align(statusPanel->obj(), LV_ALIGN_TOP_LEFT, 0, yPos);
    lv_obj_set_style_bg_color(statusPanel->obj(), lv_color_hex(0xE3F2FD), 0);
    lv_obj_set_style_border_width(statusPanel->obj(), 2, 0);
    lv_obj_set_style_border_color(statusPanel->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_set_style_radius(statusPanel->obj(), 8, 0);
    lv_obj_set_style_pad_all(statusPanel->obj(), 15, 0);
    
    lblStatus = new LVLabel(statusPanel);
    lblStatus->setText("📱 Settings Applied:\n• Theme: Dark Mode\n• Language: English\n• Font: Medium (18)");
    lblStatus->setFont(&lv_font_montserrat_18);
    lblStatus->setTextColor(lv_color_hex(0x90CAF9));
    lv_obj_align(lblStatus->obj(), LV_ALIGN_TOP_LEFT, 0, 0);
    
    ESP_LOGI(TAG, "✅ Dropdown Selection UI created");
    ESP_LOGI(TAG, "\n📊 Initial state:");
    ESP_LOGI(TAG, "   - Theme: %s", currentTheme.c_str());
    ESP_LOGI(TAG, "   - Language: %s", currentLanguage.c_str());
    ESP_LOGI(TAG, "   - Font Size: %s", currentFontSize.c_str());
    
    ESP_LOGI(TAG, "\n🧪 Manual Test Cases:");
    ESP_LOGI(TAG, "   TC1: Tap each dropdown → List expands");
    ESP_LOGI(TAG, "   TC2: Select option → List closes, value updates");
    ESP_LOGI(TAG, "   TC3: Status panel updates → Shows all selections");
    ESP_LOGI(TAG, "   TC4: Theme dropdown → Status color changes");
    ESP_LOGI(TAG, "   TC5: Language dropdown → Shows 6 options (tall list)");
    ESP_LOGI(TAG, "   TC6: Multiple dropdowns → Independent operation\n");
}
