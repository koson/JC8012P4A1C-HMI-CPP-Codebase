/**
 * @file LVEvent.cpp
 * @brief Implementation of LVEvent wrapper
 */

#include "LVEvent.hpp"
#include "esp_log.h"

static const char* TAG = "LVEvent";

// Initialize static member
std::unordered_map<lv_event_dsc_t*, std::unique_ptr<LVEventManager::CallbackData>> 
    LVEventManager::s_callbacks;

lv_event_dsc_t* LVEventManager::addEvent(lv_obj_t* obj, LVEvent::Code code,
                                         Callback callback, void* user_data)
{
    if (!obj || !callback) {
        ESP_LOGE(TAG, "Invalid parameters: obj=%p, callback=%d", obj, callback ? 1 : 0);
        return nullptr;
    }
    
    // Create callback data
    auto data = std::make_unique<CallbackData>();
    data->callback = std::move(callback);
    data->user_data = user_data;
    
    // Add LVGL event with our bridge function
    lv_event_dsc_t* dsc = lv_obj_add_event(
        obj,
        eventCallback,
        static_cast<lv_event_code_t>(code),
        data.get()  // Pass CallbackData as user_data
    );
    
    if (dsc) {
        // Store callback data indexed by descriptor
        s_callbacks[dsc] = std::move(data);
        ESP_LOGD(TAG, "Event added: obj=%p, code=%d, dsc=%p", obj, (int)code, dsc);
    } else {
        ESP_LOGE(TAG, "Failed to add event");
    }
    
    return dsc;
}

bool LVEventManager::removeEvent(lv_obj_t* obj, lv_event_dsc_t* dsc)
{
    if (!obj || !dsc) {
        return false;
    }
    
    // Remove from LVGL (descriptor API)
    bool removed = lv_obj_remove_event_dsc(obj, dsc);
    
    if (removed) {
        // Clean up our callback storage
        auto it = s_callbacks.find(dsc);
        if (it != s_callbacks.end()) {
            s_callbacks.erase(it);
            ESP_LOGD(TAG, "Event removed: dsc=%p", dsc);
        }
    }
    
    return removed;
}

void LVEventManager::removeAllEvents(lv_obj_t* obj)
{
    if (!obj) {
        return;
    }
    
    // Fallback: remove all descriptors we have registered (best-effort)
    // Note: We don't track obj per descriptor, so this clears map only.
    s_callbacks.clear();
    ESP_LOGD(TAG, "Event callbacks cleared from manager (obj=%p)", obj);
}

void LVEventManager::eventCallback(lv_event_t* e)
{
    if (!e) {
        return;
    }
    
    // Get our callback data from user_data
    CallbackData* data = static_cast<CallbackData*>(lv_event_get_user_data(e));
    
    if (!data || !data->callback) {
        ESP_LOGW(TAG, "No callback data found for event");
        return;
    }
    
    // Wrap event and call C++ callback
    LVEvent event(e);
    
    // Exceptions disabled on target; call directly
    data->callback(event);
}
