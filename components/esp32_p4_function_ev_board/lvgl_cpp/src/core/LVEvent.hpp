/**
 * @file LVEvent.hpp
 * @brief Modern C++ wrapper for LVGL event system
 * 
 * Provides type-safe event handling with lambda support, event propagation control,
 * and convenient data access methods.
 * 
 * Features:
 * - Type-safe event codes using enum class
 * - std::function callbacks with lambda support
 * - Event data accessors (target, user data, parameters)
 * - Event propagation control
 * - Custom event support
 * 
 * @example
 * // Simple click handler
 * LVEvent::addCallback(btn.get(), LVEvent::Code::Clicked, [](LVEvent& e) {
 *     printf("Button clicked!\n");
 * });
 * 
 * // Value change with data access
 * LVEvent::addCallback(slider.get(), LVEvent::Code::ValueChanged, [](LVEvent& e) {
 *     int32_t value = lv_slider_get_value((lv_obj_t*)e.getTarget());
 *     printf("New value: %d\n", value);
 * });
 * 
 * // Stop event propagation
 * LVEvent::addCallback(child.get(), LVEvent::Code::Clicked, [](LVEvent& e) {
 *     e.stopPropagation();  // Don't send to parent
 * });
 */

#pragma once

#include "lvgl.h"
#include <functional>
#include <map>
#include <memory>

/**
 * @class LVEvent
 * @brief Type-safe wrapper for LVGL event system
 * 
 * This class provides a modern C++ interface to LVGL's event system, replacing
 * raw C callbacks with std::function and enum classes for type safety.
 */
class LVEvent {
public:
    /**
     * @enum Code
     * @brief Type-safe event codes
     * 
     * Covers all standard LVGL events with meaningful names.
     */
    enum class Code {
        // Input device events
        Pressed         = LV_EVENT_PRESSED,
        Pressing        = LV_EVENT_PRESSING,
        PressLost       = LV_EVENT_PRESS_LOST,
        ShortClicked    = LV_EVENT_SHORT_CLICKED,
        LongPressed     = LV_EVENT_LONG_PRESSED,
        LongPressedRepeat = LV_EVENT_LONG_PRESSED_REPEAT,
        Clicked         = LV_EVENT_CLICKED,
        Released        = LV_EVENT_RELEASED,
        
        // Scroll events
        ScrollBegin     = LV_EVENT_SCROLL_BEGIN,
        ScrollEnd       = LV_EVENT_SCROLL_END,
        Scroll          = LV_EVENT_SCROLL,
        
        // Focus events
        Focused         = LV_EVENT_FOCUSED,
        Defocused       = LV_EVENT_DEFOCUSED,
        FocusedKey      = LV_EVENT_KEY,
        
        // Value change
        ValueChanged    = LV_EVENT_VALUE_CHANGED,
        
        // Drawing events
        RefreshExt      = LV_EVENT_REFR_EXT_DRAW_SIZE,
        DrawMain        = LV_EVENT_DRAW_MAIN,
        DrawMainBegin   = LV_EVENT_DRAW_MAIN_BEGIN,
        DrawMainEnd     = LV_EVENT_DRAW_MAIN_END,
        DrawPost        = LV_EVENT_DRAW_POST,
        DrawPostBegin   = LV_EVENT_DRAW_POST_BEGIN,
        DrawPostEnd     = LV_EVENT_DRAW_POST_END,
        DrawPartBegin   = LV_EVENT_DRAW_PART_BEGIN,
        DrawPartEnd     = LV_EVENT_DRAW_PART_END,
        
        // Other events
        Ready           = LV_EVENT_READY,
        Cancel          = LV_EVENT_CANCEL,
        Delete          = LV_EVENT_DELETE,
        ChildChanged    = LV_EVENT_CHILD_CHANGED,
        ChildCreated    = LV_EVENT_CHILD_CREATED,
        ChildDeleted    = LV_EVENT_CHILD_DELETED,
        SizeChanged     = LV_EVENT_SIZE_CHANGED,
        StyleChanged    = LV_EVENT_STYLE_CHANGED,
        LayoutChanged   = LV_EVENT_LAYOUT_CHANGED,
        GetSelfSize     = LV_EVENT_GET_SELF_SIZE,
        
        // Screen events
        ScreenLoaded    = LV_EVENT_SCREEN_LOADED,
        ScreenUnloaded  = LV_EVENT_SCREEN_UNLOADED,
        ScreenLoadStart = LV_EVENT_SCREEN_LOAD_START,
        ScreenUnloadStart = LV_EVENT_SCREEN_UNLOAD_START,
        
        // Special
        All             = LV_EVENT_ALL
    };

    /**
     * @brief Callback function type
     * 
     * Callbacks receive LVEvent reference for accessing event data.
     */
    using Callback = std::function<void(LVEvent&)>;

    /**
     * @brief Construct event wrapper from LVGL event
     * @param event Raw LVGL event pointer
     */
    explicit LVEvent(lv_event_t* event) : m_event(event) {}

    /**
     * @brief Add event callback to object
     * @param obj Target object
     * @param code Event code to listen for
     * @param callback Function to call when event occurs
     * @return Event descriptor for later removal (or nullptr if failed)
     * 
     * @example
     * LVEvent::addCallback(btn.get(), LVEvent::Code::Clicked, [](LVEvent& e) {
     *     printf("Clicked!\n");
     * });
     */
    static lv_event_dsc_t* addCallback(lv_obj_t* obj, Code code, Callback callback);

    /**
     * @brief Remove event callback
     * @param obj Target object
     * @param dsc Event descriptor from addCallback
     * @return true if removed successfully
     */
    static bool removeCallback(lv_obj_t* obj, lv_event_dsc_t* dsc);

    /**
     * @brief Send custom event to object
     * @param obj Target object
     * @param code Event code
     * @param param Optional parameter data
     * @return Result code from LVGL
     */
    static lv_result_t sendEvent(lv_obj_t* obj, Code code, void* param = nullptr);

    // ========== Event Data Accessors ==========

    /**
     * @brief Get event code
     * @return Event code enum
     */
    Code getCode() const {
        return static_cast<Code>(lv_event_get_code(m_event));
    }

    /**
     * @brief Get target object (object that originally received the event)
     * @return Object pointer
     */
    lv_obj_t* getTarget() const {
        return lv_event_get_target(m_event);
    }

    /**
     * @brief Get current target (may differ from target during bubbling)
     * @return Object pointer
     */
    lv_obj_t* getCurrentTarget() const {
        return lv_event_get_current_target(m_event);
    }

    /**
     * @brief Get user data attached to event
     * @return User data pointer
     */
    void* getUserData() const {
        return lv_event_get_user_data(m_event);
    }

    /**
     * @brief Get event parameter
     * @return Parameter pointer
     */
    void* getParam() const {
        return lv_event_get_param(m_event);
    }

    /**
     * @brief Get raw LVGL event pointer
     * @return lv_event_t pointer
     */
    lv_event_t* getRaw() const {
        return m_event;
    }

    // ========== Event Control ==========

    /**
     * @brief Stop event from propagating to parent objects
     * 
     * Call this in a callback to prevent the event from bubbling up
     * the widget hierarchy.
     */
    void stopPropagation() {
        lv_event_stop_bubbling(m_event);
    }

    /**
     * @brief Stop further processing of this event
     * 
     * Prevents other callbacks from being called for this event.
     */
    void stopProcessing() {
        lv_event_stop_processing(m_event);
    }

    // ========== Helper Methods ==========

    /**
     * @brief Check if event is a specific code
     * @param code Event code to check
     * @return true if matches
     */
    bool isCode(Code code) const {
        return getCode() == code;
    }

    /**
     * @brief Get typed parameter (convenience template)
     * @tparam T Parameter type
     * @return Typed pointer to parameter
     * 
     * @example
     * int32_t* value = e.getParamAs<int32_t>();
     */
    template<typename T>
    T* getParamAs() const {
        return static_cast<T*>(getParam());
    }

    /**
     * @brief Get typed user data (convenience template)
     * @tparam T User data type
     * @return Typed pointer to user data
     */
    template<typename T>
    T* getUserDataAs() const {
        return static_cast<T*>(getUserData());
    }

private:
    lv_event_t* m_event;  ///< Raw LVGL event pointer

    /**
     * @brief Storage for callback functions
     * 
     * Maps event descriptor to callback function to keep callbacks alive.
     */
    static std::map<lv_event_dsc_t*, std::shared_ptr<Callback>> s_callbacks;

    /**
     * @brief Static C callback that bridges to C++ std::function
     * @param event Raw LVGL event
     */
    static void eventCallback(lv_event_t* event);
};
