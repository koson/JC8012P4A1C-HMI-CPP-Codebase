#include "ScreenManager.hpp"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "ScreenManager";

ScreenManager& ScreenManager::getInstance()
{
    static ScreenManager instance;
    return instance;
}

ScreenManager::ScreenManager()
    : current_screen("")
    , default_push_transition(LVScreen::Transition::MoveLeft)
    , default_pop_transition(LVScreen::Transition::MoveRight)
    , default_duration(300)
{
    ESP_LOGI(TAG, "ScreenManager initialized");
}

// === Screen Registration ===

void ScreenManager::registerScreen(const std::string& name, LVScreen* screen)
{
    if (!screen) {
        ESP_LOGE(TAG, "Cannot register null screen: %s", name.c_str());
        return;
    }
    
    if (screens.find(name) != screens.end()) {
        ESP_LOGW(TAG, "Screen '%s' already registered, replacing", name.c_str());
    }
    
    screens[name] = screen;
    ESP_LOGI(TAG, "Registered screen: %s", name.c_str());
}

void ScreenManager::unregisterScreen(const std::string& name)
{
    auto it = screens.find(name);
    if (it != screens.end()) {
        screens.erase(it);
        ESP_LOGI(TAG, "Unregistered screen: %s", name.c_str());
        
        // Remove from history if present
        history.erase(std::remove(history.begin(), history.end(), name), history.end());
        
        // Clear current if it was this screen
        if (current_screen == name) {
            current_screen = "";
        }
    }
}

LVScreen* ScreenManager::getScreen(const std::string& name) const
{
    auto it = screens.find(name);
    return (it != screens.end()) ? it->second : nullptr;
}

bool ScreenManager::hasScreen(const std::string& name) const
{
    return screens.find(name) != screens.end();
}

// === Navigation ===

bool ScreenManager::navigateTo(const std::string& name, 
                               LVScreen::Transition transition,
                               uint32_t duration)
{
    LVScreen* screen = getScreen(name);
    if (!screen) {
        ESP_LOGE(TAG, "Screen not found: %s", name.c_str());
        return false;
    }
    
    // Clear history and navigate
    clearHistory();
    screen->load(transition, duration);
    current_screen = name;
    
    ESP_LOGI(TAG, "Navigated to: %s (history cleared)", name.c_str());
    return true;
}

bool ScreenManager::push(const std::string& name,
                         LVScreen::Transition transition,
                         uint32_t duration)
{
    LVScreen* screen = getScreen(name);
    if (!screen) {
        ESP_LOGE(TAG, "Screen not found: %s", name.c_str());
        return false;
    }
    
    // Add current screen to history before pushing new one
    if (!current_screen.empty()) {
        history.push_back(current_screen);
    }
    
    // Load new screen
    screen->load(transition, duration);
    current_screen = name;
    
    ESP_LOGI(TAG, "Pushed screen: %s (history size: %d)", 
             name.c_str(), history.size());
    return true;
}

bool ScreenManager::back(LVScreen::Transition transition, uint32_t duration)
{
    if (history.empty()) {
        ESP_LOGW(TAG, "No history to go back to");
        return false;
    }
    
    // Get previous screen from history
    std::string prev_name = history.back();
    history.pop_back();
    
    LVScreen* screen = getScreen(prev_name);
    if (!screen) {
        ESP_LOGE(TAG, "Previous screen not found: %s", prev_name.c_str());
        return false;
    }
    
    // Load previous screen
    screen->load(transition, duration);
    current_screen = prev_name;
    
    ESP_LOGI(TAG, "Back to: %s (history size: %d)", 
             prev_name.c_str(), history.size());
    return true;
}

bool ScreenManager::popTo(const std::string& name,
                          LVScreen::Transition transition,
                          uint32_t duration)
{
    // Find screen in history
    auto it = std::find(history.begin(), history.end(), name);
    if (it == history.end()) {
        ESP_LOGE(TAG, "Screen not in history: %s", name.c_str());
        return false;
    }
    
    // Remove all screens after the target
    history.erase(it + 1, history.end());
    
    // Remove target from history (it will become current)
    history.erase(it);
    
    LVScreen* screen = getScreen(name);
    if (!screen) {
        ESP_LOGE(TAG, "Screen not found: %s", name.c_str());
        return false;
    }
    
    // Load target screen
    screen->load(transition, duration);
    current_screen = name;
    
    ESP_LOGI(TAG, "Popped to: %s (history size: %d)", 
             name.c_str(), history.size());
    return true;
}

// === History Management ===

void ScreenManager::clearHistory()
{
    history.clear();
    ESP_LOGI(TAG, "History cleared");
}

std::string ScreenManager::getCurrentScreenName() const
{
    return current_screen;
}

LVScreen* ScreenManager::getCurrentScreen() const
{
    return getScreen(current_screen);
}

size_t ScreenManager::getHistorySize() const
{
    return history.size();
}

bool ScreenManager::canGoBack() const
{
    return !history.empty();
}

std::string ScreenManager::getHistoryAt(size_t index) const
{
    if (index >= history.size()) {
        return "";
    }
    return history[index];
}

// === Settings ===

void ScreenManager::setDefaultTransitions(LVScreen::Transition pushTrans,
                                         LVScreen::Transition popTrans)
{
    default_push_transition = pushTrans;
    default_pop_transition = popTrans;
    ESP_LOGI(TAG, "Default transitions set");
}

void ScreenManager::setDefaultDuration(uint32_t ms)
{
    default_duration = ms;
    ESP_LOGI(TAG, "Default duration set: %lu ms", ms);
}
