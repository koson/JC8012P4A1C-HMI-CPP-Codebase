/**
 * @file LVView.cpp
 * @brief View base class implementation
 */

#include "LVView.hpp"
#include "esp_log.h"

static const char* TAG = "LVView";

LVView::LVView(const char* className)
    : LVScreen(nullptr)
    , m_document(nullptr)
    , m_active(false)
    , m_initialized(false)
    , m_className(className ? className : "LVView")
{
    ESP_LOGI(TAG, "View created: %s", className);
}

LVView::~LVView()
{
    ESP_LOGI(TAG, "View destroyed");
    
    // Deactivate if active
    if (m_active) {
        onDeactivate();
    }
}

void LVView::setDocument(LVDocument* doc)
{
    if (m_document != doc) {
        ESP_LOGI(TAG, "Setting document: %p", doc);
        
        m_document = doc;
        
        // If view is already initialized, update it
        if (m_initialized) {
            updateView();
        }
    }
}

void LVView::onInitialUpdate()
{
    ESP_LOGI(TAG, "Initial update");
    
    if (!m_initialized) {
        m_initialized = true;
        
        // Create default UI if not overridden
        createDefaultUI();
        
        // Update from document if available
        if (m_document) {
            onUpdate(m_document, nullptr);
        }
    }
}

void LVView::onUpdate(LVDocument* doc, void* hint)
{
    // Default implementation does nothing
    // Derived classes override to update UI from document
    ESP_LOGD(TAG, "Update (hint=%p)", hint);
}

void LVView::onActivate()
{
    ESP_LOGI(TAG, "View activated");
    m_active = true;
    
    // Perform initial update if not done yet
    if (!m_initialized) {
        onInitialUpdate();
    }
}

void LVView::onDeactivate()
{
    ESP_LOGI(TAG, "View deactivated");
    m_active = false;
}

bool LVView::onDestroy()
{
    ESP_LOGI(TAG, "View destroying");
    
    // Default implementation allows destruction
    // Derived classes can override to prompt for save, etc.
    return true;
}

void LVView::updateView(void* hint)
{
    if (m_document) {
        onUpdate(m_document, hint);
    }
}

void LVView::setDocumentModified(bool modified)
{
    if (m_document) {
        m_document->setModified(modified);
    }
}

void LVView::createDefaultUI()
{
    // Default implementation - empty
    // Override onInitialUpdate() to create custom UI
    ESP_LOGI(TAG, "createDefaultUI called - override onInitialUpdate() for custom UI");
}
