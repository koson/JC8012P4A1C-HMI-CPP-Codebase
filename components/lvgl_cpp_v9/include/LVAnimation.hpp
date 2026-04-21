#pragma once

#include "lvgl.h"
#include <functional>

/**
 * @brief C++ wrapper for LVGL animations
 * 
 * Provides smooth property animations with:
 * - Position (X, Y)
 * - Size (Width, Height)
 * - Opacity
 * - Rotation
 * - Custom values
 * 
 * Features:
 * - Duration and delay control
 * - Easing functions
 * - Repeat with playback
 * - Callbacks (start, ready, deleted)
 * - Method chaining
 * - RAII cleanup
 */
class LVAnimation {
public:
    // Animation properties
    enum class Property {
        X,           // X position
        Y,           // Y position
        Width,       // Widget width
        Height,      // Widget height
        Opacity,     // Opacity (0-255)
        Rotation,    // Rotation angle (0-3600 = 0-360°)
        Custom       // Custom property with callback
    };

    // Easing/path types
    enum class Path {
        Linear,           // Constant speed
        EaseIn,          // Slow start
        EaseOut,         // Slow end
        EaseInOut,       // Slow start and end
        Overshoot,       // Overshoot the end value
        Bounce           // Bounce at the end
    };

    /**
     * @brief Create animation for a target object
     * @param target LVGL object to animate
     */
    explicit LVAnimation(lv_obj_t* target);
    
    /**
     * @brief Destructor - stops and deletes animation
     */
    ~LVAnimation();

    // === Configuration (Method Chaining) ===
    
    /**
     * @brief Set property to animate
     * @param prop Property type
     * @return Reference for chaining
     */
    LVAnimation& setProperty(Property prop);
    
    /**
     * @brief Set start and end values
     * @param start Starting value
     * @param end Ending value
     * @return Reference for chaining
     */
    LVAnimation& setValues(int32_t start, int32_t end);
    
    /**
     * @brief Set animation duration
     * @param ms Duration in milliseconds
     * @return Reference for chaining
     */
    LVAnimation& setDuration(uint32_t ms);
    
    /**
     * @brief Set delay before animation starts
     * @param ms Delay in milliseconds
     * @return Reference for chaining
     */
    LVAnimation& setDelay(uint32_t ms);
    
    /**
     * @brief Set easing/path function
     * @param path Path type
     * @return Reference for chaining
     */
    LVAnimation& setPath(Path path);
    
    /**
     * @brief Set repeat count (-1 = infinite)
     * @param count Number of repeats
     * @return Reference for chaining
     */
    LVAnimation& setRepeatCount(int32_t count);
    
    /**
     * @brief Set delay between repeats
     * @param ms Delay in milliseconds
     * @return Reference for chaining
     */
    LVAnimation& setRepeatDelay(uint32_t ms);
    
    /**
     * @brief Enable playback (reverse animation)
     * @param enable True to enable
     * @return Reference for chaining
     */
    LVAnimation& setPlayback(bool enable);
    
    /**
     * @brief Set playback delay
     * @param ms Delay before playback in milliseconds
     * @return Reference for chaining
     */
    LVAnimation& setPlaybackDelay(uint32_t ms);

    // === Custom Property ===
    
    /**
     * @brief Set custom animation callback
     * @param callback Function called on each animation step
     * @return Reference for chaining
     */
    LVAnimation& setCustomCallback(std::function<void(int32_t)> callback);

    // === Callbacks ===
    
    /**
     * @brief Set callback when animation starts
     * @param callback Function to call
     * @return Reference for chaining
     */
    LVAnimation& onStart(std::function<void()> callback);
    
    /**
     * @brief Set callback when animation completes
     * @param callback Function to call
     * @return Reference for chaining
     */
    LVAnimation& onReady(std::function<void()> callback);

    // === Control ===
    
    /**
     * @brief Start the animation
     */
    void start();
    
    /**
     * @brief Stop the animation
     */
    void stop();
    
    /**
     * @brief Get underlying LVGL animation handle
     */
    lv_anim_t* getAnim() { return &anim; }

private:
    lv_anim_t anim;
    lv_obj_t* target_obj;
    Property current_property;
    uint32_t duration_ms;
    
    // User callbacks
    std::function<void(int32_t)> custom_exec_cb;
    std::function<void()> start_cb;
    std::function<void()> ready_cb;
    
    // Static callbacks for LVGL
    static void exec_cb_wrapper(void* var, int32_t value);
    static void start_cb_wrapper(lv_anim_t* a);
    static void ready_cb_wrapper(lv_anim_t* a);
    static void deleted_cb_wrapper(lv_anim_t* a);
    
    // Helper to get property setter
    void setupPropertyExec();
};
