#pragma once

#include "lvgl.h"
#include <functional>

/**
 * @brief C++ wrapper for LVGL screen objects
 * 
 * Provides screen management with:
 * - Automatic screen creation
 * - Transition effects
 * - Lifecycle callbacks
 * - RAII cleanup
 * 
 * Usage:
 * @code
 * LVScreen* mainScreen = new LVScreen();
 * mainScreen->onLoad([]() { ESP_LOGI("Screen", "Main screen loaded"); })
 *           .load(LVScreen::Transition::FadeIn, 300);
 * @endcode
 */
class LVScreen {
public:
    // Screen transition types
    enum class Transition {
        None,               // No transition
        FadeIn,            // Fade in effect
        FadeOut,           // Fade out effect
        MoveLeft,          // Move from right to left
        MoveRight,         // Move from left to right
        MoveTop,           // Move from bottom to top
        MoveBottom,        // Move from top to bottom
        OverLeft,          // New screen slides over from left
        OverRight,         // New screen slides over from right
        OverTop,           // New screen slides over from top
        OverBottom         // New screen slides over from bottom
    };

    /**
     * @brief Create a new screen
     * @param parent Optional parent object (nullptr for independent screen)
     */
    explicit LVScreen(lv_obj_t* parent = nullptr);
    
    /**
     * @brief Destructor - cleans up screen if not currently active
     */
    ~LVScreen();

    // === Screen Management ===
    
    /**
     * @brief Load this screen with transition effect
     * @param transition Transition type
     * @param duration Transition duration in milliseconds
     */
    void load(Transition transition = Transition::None, uint32_t duration = 0);
    
    /**
     * @brief Check if this is the currently active screen
     */
    bool isActive() const;
    
    /**
     * @brief Get the underlying LVGL screen object
     */
    lv_obj_t* getObj() const { return screen; }

    // === Lifecycle Callbacks ===
    
    /**
     * @brief Set callback when screen is about to load
     * @param callback Function to call before screen loads
     * @return Reference for chaining
     */
    LVScreen& onLoad(std::function<void()> callback);
    
    /**
     * @brief Set callback when screen is unloaded
     * @param callback Function to call when screen is unloaded
     * @return Reference for chaining
     */
    LVScreen& onUnload(std::function<void()> callback);

    // === Screen Configuration ===
    
    /**
     * @brief Set background color
     * @param color Background color
     * @return Reference for chaining
     */
    LVScreen& setBackgroundColor(lv_color_t color);
    
    /**
     * @brief Set background opacity
     * @param opa Opacity value (0-255)
     * @return Reference for chaining
     */
    LVScreen& setBackgroundOpacity(lv_opa_t opa);
    
    /**
     * @brief Enable/disable scrolling
     * @param enable True to enable scrolling
     * @return Reference for chaining
     */
    LVScreen& setScrollable(bool enable);
    
    /**
     * @brief Clean all children from screen
     */
    void clean();

private:
    lv_obj_t* screen;
    
    // Lifecycle callbacks
    std::function<void()> load_cb;
    std::function<void()> unload_cb;
    
    // Track if we created the screen (for cleanup)
    bool owns_screen;
    
    // Helper to convert transition enum to LVGL constant
    static lv_screen_load_anim_t getTransitionType(Transition transition);
};
