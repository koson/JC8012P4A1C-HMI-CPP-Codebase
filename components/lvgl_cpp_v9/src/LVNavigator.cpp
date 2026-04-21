/**
 * @file LVNavigator.cpp
 * @brief Navigation stack implementation
 */

#include "LVNavigator.hpp"
#include "esp_log.h"
#include <algorithm>

static const char* TAG = "LVNavigator";

LVNavigator& LVNavigator::getInstance()
{
    static LVNavigator instance;
    return instance;
}

LVNavigator::LVNavigator()
{
    ESP_LOGI(TAG, "Navigator created");
}

LVNavigator::~LVNavigator()
{
    ESP_LOGI(TAG, "Navigator destroyed");
    clearStack();
}

bool LVNavigator::push(LVView* view, bool animated)
{
    if (!view) {
        ESP_LOGE(TAG, "Cannot push null view");
        return false;
    }
    
    ESP_LOGI(TAG, "Pushing view: %s (stack depth: %d -> %d)", 
             view->getClassName(), m_viewStack.size(), m_viewStack.size() + 1);
    
    // Deactivate current view
    if (!m_viewStack.empty()) {
        deactivateView(m_viewStack.back().get());
    }
    
    // Add to stack
    m_viewStack.push_back(std::unique_ptr<LVView>(view));
    
    // Activate new view
    activateView(view);
    
    return true;
}

bool LVNavigator::pop(bool animated)
{
    if (m_viewStack.size() <= 1) {
        ESP_LOGW(TAG, "Cannot pop root view");
        return false;
    }
    
    ESP_LOGI(TAG, "Popping view (stack depth: %d -> %d)", 
             m_viewStack.size(), m_viewStack.size() - 1);
    
    // Get current view
    LVView* currentView = m_viewStack.back().get();
    
    // Check if view allows destruction
    if (!currentView->onDestroy()) {
        ESP_LOGI(TAG, "View cancelled destruction");
        return false;
    }
    
    // Deactivate current view
    deactivateView(currentView);
    
    // Remove from stack (destroys view)
    m_viewStack.pop_back();
    
    // Activate previous view
    if (!m_viewStack.empty()) {
        activateView(m_viewStack.back().get());
    }
    
    return true;
}

bool LVNavigator::popTo(LVView* view, bool animated)
{
    if (!view) {
        ESP_LOGE(TAG, "Cannot pop to null view");
        return false;
    }
    
    // Find view in stack
    auto it = std::find_if(m_viewStack.begin(), m_viewStack.end(),
        [view](const std::unique_ptr<LVView>& v) { return v.get() == view; });
    
    if (it == m_viewStack.end()) {
        ESP_LOGE(TAG, "View not found in stack");
        return false;
    }
    
    // Calculate how many to pop
    size_t targetIndex = std::distance(m_viewStack.begin(), it);
    size_t popCount = m_viewStack.size() - targetIndex - 1;
    
    if (popCount == 0) {
        ESP_LOGW(TAG, "Already at target view");
        return true;
    }
    
    ESP_LOGI(TAG, "Popping to view at index %d (removing %d views)", targetIndex, popCount);
    
    // Deactivate current view
    deactivateView(m_viewStack.back().get());
    
    // Remove views from stack
    m_viewStack.erase(it + 1, m_viewStack.end());
    
    // Activate target view
    activateView(view);
    
    return true;
}

bool LVNavigator::popToRoot(bool animated)
{
    if (m_viewStack.empty()) {
        ESP_LOGW(TAG, "Stack is empty");
        return false;
    }
    
    return popTo(m_viewStack.front().get(), animated);
}

bool LVNavigator::replace(LVView* view, bool animated)
{
    if (!view) {
        ESP_LOGE(TAG, "Cannot replace with null view");
        return false;
    }
    
    if (m_viewStack.empty()) {
        ESP_LOGI(TAG, "Stack empty, setting as root");
        return setRootView(view);
    }
    
    ESP_LOGI(TAG, "Replacing current view");
    
    // Deactivate current view
    deactivateView(m_viewStack.back().get());
    
    // Replace in stack
    m_viewStack.back().reset(view);
    
    // Activate new view
    activateView(view);
    
    return true;
}

LVView* LVNavigator::getCurrentView() const
{
    return m_viewStack.empty() ? nullptr : m_viewStack.back().get();
}

LVView* LVNavigator::getRootView() const
{
    return m_viewStack.empty() ? nullptr : m_viewStack.front().get();
}

bool LVNavigator::isInStack(LVView* view) const
{
    if (!view) return false;
    
    return std::any_of(m_viewStack.begin(), m_viewStack.end(),
        [view](const std::unique_ptr<LVView>& v) { return v.get() == view; });
}

void LVNavigator::clearStack()
{
    ESP_LOGI(TAG, "Clearing navigation stack (%d views)", m_viewStack.size());
    
    // Deactivate current view
    if (!m_viewStack.empty()) {
        deactivateView(m_viewStack.back().get());
    }
    
    // Clear stack (destroys all views)
    m_viewStack.clear();
}

bool LVNavigator::setRootView(LVView* view)
{
    if (!view) {
        ESP_LOGE(TAG, "Cannot set null root view");
        return false;
    }
    
    ESP_LOGI(TAG, "Setting root view: %s", view->getClassName());
    
    // Clear existing stack
    clearStack();
    
    // Add as root
    m_viewStack.push_back(std::unique_ptr<LVView>(view));
    
    // Activate root view
    activateView(view);
    
    return true;
}

void LVNavigator::activateView(LVView* view)
{
    if (!view) return;
    
    ESP_LOGI(TAG, "Activating view: %s", view->getClassName());
    
    // Load screen
    view->load();
    
    // Call lifecycle hook
    view->onActivate();
}

void LVNavigator::deactivateView(LVView* view)
{
    if (!view) return;
    
    ESP_LOGI(TAG, "Deactivating view: %s", view->getClassName());
    
    // Call lifecycle hook
    view->onDeactivate();
}

void LVNavigator::removeViewAt(size_t index)
{
    if (index >= m_viewStack.size()) {
        ESP_LOGE(TAG, "Invalid stack index: %d", index);
        return;
    }
    
    // Deactivate if current
    if (index == m_viewStack.size() - 1) {
        deactivateView(m_viewStack[index].get());
    }
    
    // Remove from stack
    m_viewStack.erase(m_viewStack.begin() + index);
}
