/**
 * @file LVDocument.cpp
 * @brief Document base class implementation
 */

#include "LVDocument.hpp"
#include "LVStorage.hpp"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "LVDocument";

LVDocument::LVDocument(const char* className)
    : m_modified(false)
    , m_className(className ? className : "LVDocument")
    , m_modifiedCallback(nullptr)
    , m_modifiedCallbackUserData(nullptr)
{
    ESP_LOGI(TAG, "Document created: %s", m_className);
}

LVDocument::~LVDocument()
{
    ESP_LOGI(TAG, "Document destroyed");
}

bool LVDocument::newDocument()
{
    ESP_LOGI(TAG, "Creating new document");
    
    // Clear file path (untitled)
    m_filePath.clear();
    m_modified = false;
    
    // Call derived class hook
    onNewDocument();
    
    return true;
}

bool LVDocument::open(const char* filePath)
{
    if (!filePath) {
        ESP_LOGE(TAG, "Invalid file path");
        return false;
    }
    
    ESP_LOGI(TAG, "Opening document: %s", filePath);
    
    // Get storage instance
    LVStorage* storage = getStorage();
    if (!storage || !storage->isReady()) {
        ESP_LOGE(TAG, "Storage not available");
        return false;
    }
    
    // Read file content
    std::vector<uint8_t> fileData;
    if (!storage->readFile(filePath, fileData)) {
        ESP_LOGE(TAG, "Failed to read file");
        return false;
    }
    std::string data(fileData.begin(), fileData.end());
    
    // Deserialize data
    if (!serialize(true, data)) {
        ESP_LOGE(TAG, "Failed to deserialize document");
        return false;
    }
    
    // Update state
    m_filePath = filePath;
    m_modified = false;
    
    // Call derived class hook
    onDocumentOpened();
    
    ESP_LOGI(TAG, "Document opened successfully");
    return true;
}

bool LVDocument::save()
{
    if (isUntitled()) {
        ESP_LOGW(TAG, "Cannot save untitled document, use saveAs()");
        return false;
    }
    
    return saveAs(m_filePath.c_str());
}

bool LVDocument::saveAs(const char* filePath)
{
    if (!filePath) {
        ESP_LOGE(TAG, "Invalid file path");
        return false;
    }
    
    ESP_LOGI(TAG, "Saving document as: %s", filePath);
    
    // Get storage instance
    LVStorage* storage = getStorage();
    if (!storage || !storage->isReady()) {
        ESP_LOGE(TAG, "Storage not available");
        return false;
    }
    
    // Call pre-save hook
    onDocumentSaving();
    
    // Serialize data
    std::string data;
    if (!serialize(false, data)) {
        ESP_LOGE(TAG, "Failed to serialize document");
        return false;
    }
    
    // Write to file
    if (!storage->writeFile(filePath, reinterpret_cast<const uint8_t*>(data.c_str()), data.size())) {
        ESP_LOGE(TAG, "Failed to write file");
        return false;
    }
    
    // Update state
    m_filePath = filePath;
    m_modified = false;
    
    ESP_LOGI(TAG, "Document saved successfully");
    return true;
}

bool LVDocument::close(bool force)
{
    ESP_LOGI(TAG, "Closing document (force=%d)", force);
    
    // Check if modified and not forced
    if (!force && m_modified) {
        ESP_LOGW(TAG, "Document has unsaved changes");
        // In a real app, prompt user to save
        // For now, just warn
    }
    
    // Call derived class hook (can cancel close)
    if (!onDocumentClosing()) {
        ESP_LOGI(TAG, "Document close cancelled");
        return false;
    }
    
    // Reset state
    m_filePath.clear();
    m_modified = false;
    
    ESP_LOGI(TAG, "Document closed");
    return true;
}

bool LVDocument::serialize(bool isLoading, std::string& data)
{
    // Base implementation does nothing
    // Derived classes must override this
    ESP_LOGW(TAG, "serialize() not implemented in derived class");
    return true;
}

void LVDocument::setModified(bool modified)
{
    if (m_modified != modified) {
        m_modified = modified;
        
        ESP_LOGD(TAG, "Document modified state changed: %d", modified);
        
        // Trigger callback
        if (m_modifiedCallback) {
            m_modifiedCallback(this, m_modifiedCallbackUserData);
        }
    }
}

const char* LVDocument::getTitle() const
{
    updateTitle();
    return m_title.c_str();
}

void LVDocument::setModifiedCallback(std::function<void(LVDocument*, void*)> callback, void* userData)
{
    m_modifiedCallback = callback;
    m_modifiedCallbackUserData = userData;
}

void LVDocument::onNewDocument()
{
    // Default implementation does nothing
}

void LVDocument::onDocumentOpened()
{
    // Default implementation does nothing
}

void LVDocument::onDocumentSaving()
{
    // Default implementation does nothing
}

bool LVDocument::onDocumentClosing()
{
    // Default implementation allows closing
    return true;
}

LVStorage* LVDocument::getStorage() const
{
    return &LVStorage::getInstance();
}

void LVDocument::updateTitle() const
{
    if (isUntitled()) {
        m_title = "Untitled";
    } else {
        // Extract filename from path
        const char* lastSlash = strrchr(m_filePath.c_str(), '/');
        if (lastSlash) {
            m_title = lastSlash + 1;
        } else {
            m_title = m_filePath;
        }
    }
}
