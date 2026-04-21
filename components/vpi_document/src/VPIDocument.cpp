#include "VPIDocument.h"
#include "esp_log.h"
#include <algorithm>
#include <cstring>
#include <sys/time.h>

static const char* TAG = "VPIDocument";

// ========== Helper Functions ==========

extern "C" const char* vpi_state_to_string(vpi_state_t state) {
    switch (state) {
        case VPI_STATE_IDLE:         return "IDLE";
        case VPI_STATE_VACUUM:       return "VACUUM";
        case VPI_STATE_OVERPRESSURE: return "OVERPRESSURE";
        case VPI_STATE_LEAKING:      return "LEAKING";
        case VPI_STATE_COOLDOWN:     return "COOLDOWN";
        case VPI_STATE_ERROR:        return "ERROR";
        default:                     return "UNKNOWN";
    }
}

// ========== VPIDocument Implementation ==========

VPIDocument::VPIDocument() {
    // Initialize with default values
    std::memset(&current_data_, 0, sizeof(current_data_));
    current_data_.state = VPI_STATE_IDLE;
    current_data_.data_valid = false;
    
    // Set initial timestamp
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    current_data_.timestamp = tv.tv_sec;
    
    ESP_LOGI(TAG, "VPIDocument initialized");
}

VPIDocument::~VPIDocument() {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.clear();
    ESP_LOGI(TAG, "VPIDocument destroyed");
}

void VPIDocument::attach(IVPIObserver* observer) {
    if (observer == nullptr) {
        ESP_LOGW(TAG, "Cannot attach null observer");
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if already attached
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        ESP_LOGW(TAG, "Observer already attached");
        return;
    }
    
    observers_.push_back(observer);
    ESP_LOGI(TAG, "Observer attached (total: %d)", observers_.size());
    
    // Immediately notify new observer with current data
    if (current_data_.data_valid) {
        observer->onDataUpdate(current_data_);
    }
}

void VPIDocument::detach(IVPIObserver* observer) {
    if (observer == nullptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
        ESP_LOGI(TAG, "Observer detached (remaining: %d)", observers_.size());
    }
}

void VPIDocument::updateData(const vpi_data_snapshot_t& snapshot) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_data_ = snapshot;
        current_data_.data_valid = true;
        
        // Update timestamp if not set
        if (current_data_.timestamp == 0) {
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            current_data_.timestamp = tv.tv_sec;
        }
    }
    
    notifyObservers();
}

vpi_data_snapshot_t VPIDocument::getCurrentData() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_data_;
}

void VPIDocument::updateState(vpi_state_t state) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (current_data_.state != state) {
            ESP_LOGI(TAG, "State changed: %s -> %s", 
                     vpi_state_to_string(current_data_.state),
                     vpi_state_to_string(state));
            
            current_data_.state = state;
            
            // Update timestamp
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            current_data_.timestamp = tv.tv_sec;
        }
    }
    
    notifyObservers();
}

void VPIDocument::updatePM2230(const pm2230_data_t& pm2230) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_data_.pm2230 = pm2230;
        current_data_.data_valid = true;
        
        // Update timestamp
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        current_data_.timestamp = tv.tv_sec;
    }
    
    notifyObservers();
}

void VPIDocument::updatePressure(const pressure_data_t& pressure) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_data_.pressure = pressure;
        current_data_.data_valid = true;
        
        // Update timestamp
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        current_data_.timestamp = tv.tv_sec;
    }
    
    notifyObservers();
}

void VPIDocument::notifyObservers() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!current_data_.data_valid) {
        return;
    }
    
    for (auto observer : observers_) {
        if (observer != nullptr) {
            observer->onDataUpdate(current_data_);
        }
    }
}
