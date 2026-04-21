#pragma once

#include "lvgl.h"
#include <functional>
#include <memory>

/**
 * @brief C++ wrapper for LVGL timer (lv_timer_t)
 * 
 * LVTimer provides periodic task execution with modern C++ interface.
 * Supports lambda callbacks, automatic cleanup, and RAII pattern.
 * 
 * Features:
 * - Periodic execution with configurable period
 * - Pause/resume functionality
 * - Repeat count control (finite or infinite)
 * - Automatic cleanup on destruction
 * - Type-safe callback with std::function
 * 
 * Example:
 * @code
 * // Blink LED every 500ms
 * auto blinkTimer = new LVTimer(500, [&]() {
 *     led->toggle();
 * });
 * 
 * // Auto-refresh dashboard every 1 second
 * auto refreshTimer = new LVTimer(1000, [this]() {
 *     updateDashboard();
 * });
 * 
 * // Countdown timer (10 repeats)
 * auto countdown = new LVTimer(1000, [&]() {
 *     count--;
 *     if (count == 0) timer->pause();
 * });
 * countdown->setRepeatCount(10);
 * @endcode
 */
class LVTimer {
public:
    /**
     * @brief Construct a new LVTimer
     * 
     * @param period_ms Period in milliseconds
     * @param callback Function to call on each timer tick
     */
    LVTimer(uint32_t period_ms, std::function<void()> callback);
    
    /**
     * @brief Destroy the LVTimer and cleanup LVGL resources
     */
    ~LVTimer();
    
    /**
     * @brief Pause the timer
     * 
     * The timer callback will not be called until resume() is called.
     */
    void pause();
    
    /**
     * @brief Resume a paused timer
     */
    void resume();
    
    /**
     * @brief Reset the timer
     * 
     * Resets the timer's internal counter. The next callback will be
     * called after period_ms from now.
     */
    void reset();
    
    /**
     * @brief Set the timer period
     * 
     * @param period_ms New period in milliseconds
     */
    void setPeriod(uint32_t period_ms);
    
    /**
     * @brief Get the current period
     * 
     * @return uint32_t Period in milliseconds
     */
    uint32_t getPeriod() const;
    
    /**
     * @brief Set repeat count
     * 
     * @param count Number of times to repeat (-1 = infinite)
     * 
     * After 'count' executions, the timer will automatically pause.
     * Use -1 for infinite repeating.
     */
    void setRepeatCount(int32_t count);
    
    /**
     * @brief Get repeat count
     * 
     * @return int32_t Current repeat count (-1 = infinite)
     */
    int32_t getRepeatCount() const;
    
    /**
     * @brief Check if timer is paused
     * 
     * @return true Timer is paused
     * @return false Timer is running
     */
    bool isPaused() const;
    
    /**
     * @brief Get the underlying LVGL timer object
     * 
     * Use this for advanced operations not covered by the wrapper.
     * 
     * @return lv_timer_t* Pointer to LVGL timer
     */
    lv_timer_t* timer() const { return m_timer; }
    
    /**
     * @brief Trigger the callback immediately (one-shot)
     * 
     * This calls the callback once without affecting the regular schedule.
     */
    void ready();
    
    // Delete copy constructor and assignment operator (timer is unique resource)
    LVTimer(const LVTimer&) = delete;
    LVTimer& operator=(const LVTimer&) = delete;
    
private:
    /**
     * @brief Static callback for LVGL timer
     * 
     * This bridges LVGL's C-style callback to our C++ std::function.
     * 
     * @param timer LVGL timer pointer
     */
    static void timerCallback(lv_timer_t* timer);
    
    lv_timer_t* m_timer;                    ///< LVGL timer object
    std::function<void()> m_callback;       ///< User callback function
};
