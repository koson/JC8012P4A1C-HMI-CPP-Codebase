#include "lvgl.h"
#include "ScreenManager.hpp"
#include "LVScreen.hpp"
#include "esp_log.h"

static const char* TAG = "ScreenMgrDemo";

// Forward declarations
void createHomeScreen(LVScreen* screen);
void createMenuScreen(LVScreen* screen);
void createSettingsScreen(LVScreen* screen);
void createAboutScreen(LVScreen* screen);

extern "C" void test_screen_manager_demo()
{
    ESP_LOGI(TAG, "=== ScreenManager Demo Started ===");
    
    // Get manager instance
    auto& mgr = ScreenManager::getInstance();
    
    // Create screens
    LVScreen* homeScreen = new LVScreen();
    LVScreen* menuScreen = new LVScreen();
    LVScreen* settingsScreen = new LVScreen();
    LVScreen* aboutScreen = new LVScreen();
    
    // Register screens
    mgr.registerScreen("home", homeScreen);
    mgr.registerScreen("menu", menuScreen);
    mgr.registerScreen("settings", settingsScreen);
    mgr.registerScreen("about", aboutScreen);
    
    // Create screen content
    createHomeScreen(homeScreen);
    createMenuScreen(menuScreen);
    createSettingsScreen(settingsScreen);
    createAboutScreen(aboutScreen);
    
    // Set default transitions
    mgr.setDefaultTransitions(LVScreen::Transition::MoveLeft, 
                             LVScreen::Transition::MoveRight);
    mgr.setDefaultDuration(300);
    
    // Start at home screen
    mgr.navigateTo("home", LVScreen::Transition::FadeIn, 500);
    
    ESP_LOGI(TAG, "ScreenManager demo initialized");
}

void createHomeScreen(LVScreen* screen)
{
    screen->setBackgroundColor(lv_color_hex(0x34495e));
    
    lv_obj_t* scr = screen->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "ScreenManager Demo");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    // Subtitle
    lv_obj_t* subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Home Screen");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xbdc3c7), 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_18, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 70);
    
    // Container
    lv_obj_t* container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_PCT(85), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Menu button (push to stack)
    lv_obj_t* btnMenu = lv_button_create(container);
    lv_obj_set_size(btnMenu, 280, 50);
    lv_obj_set_style_bg_color(btnMenu, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(btnMenu, 8, 0);
    
    lv_obj_t* lblMenu = lv_label_create(btnMenu);
    lv_label_set_text(lblMenu, "Open Menu (Push)");
    lv_obj_set_style_text_color(lblMenu, lv_color_white(), 0);
    lv_obj_center(lblMenu);
    
    lv_obj_add_event_cb(btnMenu, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        mgr.push("menu");
        ESP_LOGI(TAG, "Pushed to menu, history size: %d", mgr.getHistorySize());
    }, LV_EVENT_CLICKED, nullptr);
    
    // About button (navigate without history)
    lv_obj_t* btnAbout = lv_button_create(container);
    lv_obj_set_size(btnAbout, 280, 50);
    lv_obj_set_style_bg_color(btnAbout, lv_color_hex(0x9b59b6), 0);
    lv_obj_set_style_radius(btnAbout, 8, 0);
    
    lv_obj_t* lblAbout = lv_label_create(btnAbout);
    lv_label_set_text(lblAbout, "About (Navigate)");
    lv_obj_set_style_text_color(lblAbout, lv_color_white(), 0);
    lv_obj_center(lblAbout);
    
    lv_obj_add_event_cb(btnAbout, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        mgr.navigateTo("about", LVScreen::Transition::FadeIn, 400);
        ESP_LOGI(TAG, "Navigated to about (history cleared)");
    }, LV_EVENT_CLICKED, nullptr);
    
    // Info
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, "Push: Adds to history | Navigate: Clears history");
    lv_obj_set_style_text_color(info, lv_color_hex(0x95a5a6), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_12, 0);
    lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void createMenuScreen(LVScreen* screen)
{
    screen->setBackgroundColor(lv_color_hex(0x2c3e50));
    
    lv_obj_t* scr = screen->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Menu Screen");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    // History info
    lv_obj_t* histInfo = lv_label_create(scr);
    lv_obj_set_style_text_color(histInfo, lv_color_hex(0xecf0f1), 0);
    lv_obj_align(histInfo, LV_ALIGN_TOP_MID, 0, 70);
    
    // Update history display when screen loads
    screen->onLoad([histInfo]() {
        auto& mgr = ScreenManager::getInstance();
        char buf[64];
        snprintf(buf, sizeof(buf), "History: %d screens", mgr.getHistorySize());
        lv_label_set_text(histInfo, buf);
    });
    
    // Container
    lv_obj_t* container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_PCT(85), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Settings button (push)
    lv_obj_t* btnSettings = lv_button_create(container);
    lv_obj_set_size(btnSettings, 280, 50);
    lv_obj_set_style_bg_color(btnSettings, lv_color_hex(0x2ecc71), 0);
    lv_obj_set_style_radius(btnSettings, 8, 0);
    
    lv_obj_t* lblSettings = lv_label_create(btnSettings);
    lv_label_set_text(lblSettings, "Settings (Push)");
    lv_obj_set_style_text_color(lblSettings, lv_color_white(), 0);
    lv_obj_center(lblSettings);
    
    lv_obj_add_event_cb(btnSettings, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        mgr.push("settings");
        ESP_LOGI(TAG, "Pushed to settings, history size: %d", mgr.getHistorySize());
    }, LV_EVENT_CLICKED, nullptr);
    
    // Back button
    lv_obj_t* btnBack = lv_button_create(container);
    lv_obj_set_size(btnBack, 280, 50);
    lv_obj_set_style_bg_color(btnBack, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_radius(btnBack, 8, 0);
    
    lv_obj_t* lblBack = lv_label_create(btnBack);
    lv_label_set_text(lblBack, "<- Back (Pop)");
    lv_obj_set_style_text_color(lblBack, lv_color_white(), 0);
    lv_obj_center(lblBack);
    
    lv_obj_add_event_cb(btnBack, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        if (mgr.canGoBack()) {
            mgr.back();
        } else {
            ESP_LOGW(TAG, "No history to go back");
        }
    }, LV_EVENT_CLICKED, nullptr);
}

void createSettingsScreen(LVScreen* screen)
{
    screen->setBackgroundColor(lv_color_hex(0x16a085));
    
    lv_obj_t* scr = screen->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Settings Screen");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    // History info
    lv_obj_t* histInfo = lv_label_create(scr);
    lv_obj_set_style_text_color(histInfo, lv_color_white(), 0);
    lv_obj_align(histInfo, LV_ALIGN_TOP_MID, 0, 70);
    
    screen->onLoad([histInfo]() {
        auto& mgr = ScreenManager::getInstance();
        char buf[128];
        snprintf(buf, sizeof(buf), "History size: %d\nCurrent: %s", 
                 mgr.getHistorySize(), 
                 mgr.getCurrentScreenName().c_str());
        lv_label_set_text(histInfo, buf);
    });
    
    // Container
    lv_obj_t* container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_PCT(85), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Back to home button (popTo)
    lv_obj_t* btnHome = lv_button_create(container);
    lv_obj_set_size(btnHome, 280, 50);
    lv_obj_set_style_bg_color(btnHome, lv_color_hex(0xf39c12), 0);
    lv_obj_set_style_radius(btnHome, 8, 0);
    
    lv_obj_t* lblHome = lv_label_create(btnHome);
    lv_label_set_text(lblHome, "Pop to Home");
    lv_obj_set_style_text_color(lblHome, lv_color_white(), 0);
    lv_obj_center(lblHome);
    
    lv_obj_add_event_cb(btnHome, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        mgr.popTo("home");
        ESP_LOGI(TAG, "Popped to home");
    }, LV_EVENT_CLICKED, nullptr);
    
    // Back button (one level)
    lv_obj_t* btnBack = lv_button_create(container);
    lv_obj_set_size(btnBack, 280, 50);
    lv_obj_set_style_bg_color(btnBack, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_radius(btnBack, 8, 0);
    
    lv_obj_t* lblBack = lv_label_create(btnBack);
    lv_label_set_text(lblBack, "<- Back");
    lv_obj_set_style_text_color(lblBack, lv_color_white(), 0);
    lv_obj_center(lblBack);
    
    lv_obj_add_event_cb(btnBack, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        mgr.back();
    }, LV_EVENT_CLICKED, nullptr);
}

void createAboutScreen(LVScreen* screen)
{
    screen->setBackgroundColor(lv_color_hex(0x8e44ad));
    
    lv_obj_t* scr = screen->getObj();
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "About");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);
    
    // Info
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info,
        "ScreenManager Demo\n\n"
        "Features:\n"
        "- Screen registration\n"
        "- Push/Pop navigation\n"
        "- History tracking\n"
        "- popTo() to specific screen\n"
        "- Lifecycle callbacks\n\n"
        "This screen was opened\n"
        "with navigateTo()\n"
        "(history cleared)");
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(info, LV_ALIGN_CENTER, 0, -20);
    
    // Back to home button
    lv_obj_t* btnHome = lv_button_create(scr);
    lv_obj_set_size(btnHome, 280, 50);
    lv_obj_set_style_bg_color(btnHome, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(btnHome, 8, 0);
    lv_obj_align(btnHome, LV_ALIGN_BOTTOM_MID, 0, -40);
    
    lv_obj_t* lblHome = lv_label_create(btnHome);
    lv_label_set_text(lblHome, "Back to Home");
    lv_obj_set_style_text_color(lblHome, lv_color_white(), 0);
    lv_obj_center(lblHome);
    
    lv_obj_add_event_cb(btnHome, [](lv_event_t* e) {
        auto& mgr = ScreenManager::getInstance();
        mgr.navigateTo("home", LVScreen::Transition::FadeIn, 400);
    }, LV_EVENT_CLICKED, nullptr);
}
