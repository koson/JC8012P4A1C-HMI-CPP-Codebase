/**
 * @file LVEvent.cpp
 * @brief Implementation of LVEvent wrapper
 */

#include "LVEvent.hpp"

// Static storage for callbacks
std::map<lv_event_dsc_t*, std::shared_ptr<LVEvent::Callback>> LVEvent::s_callbacks;

lv_event_dsc_t* LVEvent::addCallback(lv_obj_t* obj, Code code, Callback callback) {
    if (!obj) {
        return nullptr;
    }

    // Store callback in shared_ptr for automatic memory management
    auto callback_ptr = std::make_shared<Callback>(callback);

    // Register C callback with LVGL, pass callback_ptr as user_data
    lv_event_dsc_t* dsc = lv_obj_add_event_cb(
        obj,
        eventCallback,
        static_cast<lv_event_code_t>(code),
        callback_ptr.get()
    );

    if (dsc) {
        // Store in map to keep callback alive
        s_callbacks[dsc] = callback_ptr;
    }

    return dsc;
}

bool LVEvent::removeCallback(lv_obj_t* obj, lv_event_dsc_t* dsc) {
    if (!obj || !dsc) {
        return false;
    }

    // Remove from LVGL
    bool result = lv_obj_remove_event_cb_with_user_data(obj, eventCallback, s_callbacks[dsc].get());

    // Remove from storage
    s_callbacks.erase(dsc);

    return result;
}

lv_result_t LVEvent::sendEvent(lv_obj_t* obj, Code code, void* param) {
    if (!obj) {
        return LV_RESULT_INVALID;
    }

    return lv_obj_send_event(obj, static_cast<lv_event_code_t>(code), param);
}

void LVEvent::eventCallback(lv_event_t* event) {
    // Get callback from user_data
    void* user_data = lv_event_get_user_data(event);
    if (!user_data) {
        return;
    }

    Callback* callback = static_cast<Callback*>(user_data);

    // Wrap event and call C++ callback
    LVEvent lvEvent(event);
    (*callback)(lvEvent);
}
