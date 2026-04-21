/**
 * @file LVApplication.cpp
 * @brief Application framework implementation
 */

#include "LVApplication.hpp"
#include "esp_log.h"

static const char* TAG = "LVApplication";

LVApplication& LVApplication::getInstance()
{
    static LVApplication instance;
    return instance;
}

LVApplication::LVApplication()
    : m_initialized(false)
    , m_running(false)
    , m_idleCallback(nullptr)
    , m_idleCallbackUserData(nullptr)
{
    ESP_LOGI(TAG, "Application created");
}

LVApplication::~LVApplication()
{
    ESP_LOGI(TAG, "Application destroyed");
    terminate();
}

bool LVApplication::initialize()
{
    if (m_initialized) {
        ESP_LOGW(TAG, "Application already initialized");
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing application...");
    
    // Call derived class hook
    if (!onInitialize()) {
        ESP_LOGE(TAG, "Custom initialization failed");
        return false;
    }
    
    m_initialized = true;
    m_running = true;
    
    // Call start hook
    onStart();
    
    ESP_LOGI(TAG, "Application initialized successfully");
    return true;
}

bool LVApplication::setRootView(LVView* view)
{
    if (!view) {
        ESP_LOGE(TAG, "Cannot set null root view");
        return false;
    }
    
    ESP_LOGI(TAG, "Setting root view: %s", view->getClassName());
    
    return getNavigator().setRootView(view);
}

bool LVApplication::mountSDCard(const char* mountPoint)
{
    ESP_LOGI(TAG, "Mounting SD card at %s", mountPoint);
    
    LVStorage& storage = getStorage();
    return storage.mount(LVStorage::SD_CARD, mountPoint);
}

bool LVApplication::unmountStorage()
{
    ESP_LOGI(TAG, "Unmounting storage");
    
    LVStorage& storage = getStorage();
    return storage.unmount();
}

void LVApplication::run()
{
    // Called periodically by LVGL
    // Invoke idle callback if set
    if (m_idleCallback) {
        m_idleCallback(m_idleCallbackUserData);
    }
}

void LVApplication::terminate()
{
    if (!m_running) {
        return;
    }
    
    ESP_LOGI(TAG, "Terminating application");
    
    m_running = false;
    
    // Call terminate hook
    onTerminate();
    
    // Clear navigation stack
    getNavigator().clearStack();
    
    // Unmount storage
    unmountStorage();
    
    m_initialized = false;
    
    ESP_LOGI(TAG, "Application terminated");
}

void LVApplication::setIdleCallback(std::function<void(void*)> callback, void* userData)
{
    m_idleCallback = callback;
    m_idleCallbackUserData = userData;
}

bool LVApplication::onInitialize()
{
    // Default implementation does nothing
    return true;
}

void LVApplication::onStart()
{
    ESP_LOGI(TAG, "Application started");
}

void LVApplication::onTerminate()
{
    ESP_LOGI(TAG, "Application terminating");
}
