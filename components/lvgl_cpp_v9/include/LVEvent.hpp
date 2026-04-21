/**
 * @file LVEvent.hpp
 * @brief Advanced event handling wrapper for LVGL v9
 * 
 * Provides type-safe event handling with modern C++ features:
 * - Enum class for event codes
 * - Lambda/std::function callbacks
 * - Event data accessors
 * - Propagation control
 * 
 * Phase 3.2 - Core System Classes
 */

#pragma once

#include "lvgl.h"
#include <functional>
#include <memory>
#include <unordered_map>

/**
 * @class LVEvent
 * @brief Type-safe event wrapper for LVGL
 * 
 * Usage:
 * @code
 * LVButton btn(parent);
 * btn.addEvent(LVEvent::Code::Clicked, [](LVEvent& e) {
 *     ESP_LOGI("BTN", "Button clicked!");
 * });
 * @endcode
 */
class LVEvent {
public:
    /**
     * @enum Code
     * @brief Type-safe event codes
     */
    enum class Code : uint32_t {
        // Input device events
        Pressed         = LV_EVENT_PRESSED,
        Pressing        = LV_EVENT_PRESSING,
        PressLost       = LV_EVENT_PRESS_LOST,
        ShortClicked    = LV_EVENT_SHORT_CLICKED,
        LongPressed     = LV_EVENT_LONG_PRESSED,
        LongPressedRepeat = LV_EVENT_LONG_PRESSED_REPEAT,
        Clicked         = LV_EVENT_CLICKED,
        Released        = LV_EVENT_RELEASED,
        
        // Pointer events
        ScrollBegin     = LV_EVENT_SCROLL_BEGIN,
        ScrollEnd       = LV_EVENT_SCROLL_END,
        Scroll          = LV_EVENT_SCROLL,
        Gesture         = LV_EVENT_GESTURE,
        
        // Keypad/encoder events
        Key             = LV_EVENT_KEY,
        Focused         = LV_EVENT_FOCUSED,
        Defocused       = LV_EVENT_DEFOCUSED,
        
        // Widget state events
        ValueChanged    = LV_EVENT_VALUE_CHANGED,
        Insert          = LV_EVENT_INSERT,
        Refresh         = LV_EVENT_REFRESH,
        Ready           = LV_EVENT_READY,
        Cancel          = LV_EVENT_CANCEL,
        
        // Layout events
        ChildChanged    = LV_EVENT_CHILD_CHANGED,
        ChildCreated    = LV_EVENT_CHILD_CREATED,
        ChildDeleted    = LV_EVENT_CHILD_DELETED,
        SizeChanged     = LV_EVENT_SIZE_CHANGED,
        StyleChanged    = LV_EVENT_STYLE_CHANGED,
        
        // Display events
        LayoutChanged   = LV_EVENT_LAYOUT_CHANGED,
        GetSelfSize     = LV_EVENT_GET_SELF_SIZE,
        
        // Drawing events
        Cover           = LV_EVENT_COVER_CHECK,
        RefreshExt      = LV_EVENT_REFR_EXT_DRAW_SIZE,
        DrawMain        = LV_EVENT_DRAW_MAIN,
        DrawMainBegin   = LV_EVENT_DRAW_MAIN_BEGIN,
        DrawMainEnd     = LV_EVENT_DRAW_MAIN_END,
        DrawPost        = LV_EVENT_DRAW_POST,
        DrawPostBegin   = LV_EVENT_DRAW_POST_BEGIN,
        DrawPostEnd     = LV_EVENT_DRAW_POST_END,
        
        // Special events
        DeleteObj       = LV_EVENT_DELETE,
        ScreenLoadStart = LV_EVENT_SCREEN_LOAD_START,
        ScreenLoaded    = LV_EVENT_SCREEN_LOADED,
        ScreenUnloadStart = LV_EVENT_SCREEN_UNLOAD_START,
        ScreenUnloaded  = LV_EVENT_SCREEN_UNLOADED,
        
        // All events
        All             = LV_EVENT_ALL,
    };
    
    /**
     * @brief Constructor - wraps existing lv_event_t
     * @param event Raw LVGL event pointer
     */
    explicit LVEvent(lv_event_t* event) : m_event(event) {}
    
    /**
     * @brief Get event code
     * @return Event code enum
     */
    Code getCode() const {
        return static_cast<Code>(lv_event_get_code(m_event));
    }
    
    /**
     * @brief Get target object (the object that received the event)
     * @return Target object pointer
     */
    lv_obj_t* getTarget() const {
        return static_cast<lv_obj_t*>(lv_event_get_target(m_event));
    }
    
    /**
     * @brief Get current target (may differ from target during bubbling)
     * @return Current target object pointer
     */
    lv_obj_t* getCurrentTarget() const {
        return static_cast<lv_obj_t*>(lv_event_get_current_target(m_event));
    }
    
    /**
     * @brief Get user data associated with this event
     * @return User data pointer
     */
    void* getUserData() const {
        return lv_event_get_user_data(m_event);
    }
    
    /**
     * @brief Get event parameter (event-specific data)
     * @return Parameter pointer
     */
    void* getParam() const {
        return lv_event_get_param(m_event);
    }
    
    /**
     * @brief Stop event propagation to parent objects
     */
    void stopPropagation() {
        lv_event_stop_bubbling(m_event);
    }
    
    /**
     * @brief Stop further processing of this event
     */
    void stopProcessing() {
        lv_event_stop_processing(m_event);
    }
    
    /**
     * @brief Get key value (for LV_EVENT_KEY)
     * @return Key code
     */
    uint32_t getKey() const {
        return lv_event_get_key(m_event);
    }
    
    /**
     * @brief Get scroll object (for scroll events)
     * @return Scrolled object pointer
     */
    // Scroll object helper is omitted because lv_event_get_scroll_obj may not exist in this LVGL version.
    
    /**
     * @brief Get raw LVGL event pointer
     * @return Raw event pointer
     */
    lv_event_t* getRaw() const {
        return m_event;
    }

private:
    lv_event_t* m_event;  ///< Raw LVGL event pointer
};

/**
 * @class LVEventManager
 * @brief Manages C++ callbacks for LVGL events
 * 
 * This is a helper class to bridge C-style LVGL callbacks
 * to C++ std::function callbacks.
 */
class LVEventManager {
public:
    using Callback = std::function<void(LVEvent&)>;
    
    /**
     * @brief Add event callback to object
     * @param obj Target object
     * @param code Event code to listen for
     * @param callback C++ callback function
     * @param user_data Optional user data
     * @return Event descriptor (can be used to remove callback)
     */
    static lv_event_dsc_t* addEvent(lv_obj_t* obj, LVEvent::Code code, 
                                    Callback callback, void* user_data = nullptr);
    
    /**
     * @brief Remove event callback
     * @param obj Target object
     * @param dsc Event descriptor returned by addEvent
     * @return true if removed successfully
     */
    static bool removeEvent(lv_obj_t* obj, lv_event_dsc_t* dsc);
    
    /**
     * @brief Remove all callbacks for an object
     * @param obj Target object
     */
    static void removeAllEvents(lv_obj_t* obj);

private:
    /**
     * @brief Internal C-style callback that bridges to C++
     * @param e Raw LVGL event
     */
    static void eventCallback(lv_event_t* e);
    
    /**
     * @brief Storage for callback data
     */
    struct CallbackData {
        Callback callback;
        void* user_data;
    };
    
    // Global storage for all callbacks (indexed by event descriptor)
    static std::unordered_map<lv_event_dsc_t*, std::unique_ptr<CallbackData>> s_callbacks;
};
