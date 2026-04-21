#include "LVTimer.hpp"
#include "esp_log.h"
#include "misc/lv_timer_private.h"

static const char* TAG = "LVTimer";

LVTimer::LVTimer(uint32_t period_ms, std::function<void()> callback)
    : m_timer(nullptr)
    , m_callback(callback)
{
    if (!m_callback) {
        ESP_LOGE(TAG, "Callback is null");
        return;
    }
    
    // Create LVGL timer with our static callback
    m_timer = lv_timer_create(timerCallback, period_ms, this);
    
    if (!m_timer) {
        ESP_LOGE(TAG, "Failed to create timer");
        return;
    }
    
    ESP_LOGI(TAG, "Timer created: period=%lu ms", period_ms);
}

LVTimer::~LVTimer()
{
    if (m_timer) {
        ESP_LOGI(TAG, "Deleting timer");
        lv_timer_delete(m_timer);
        m_timer = nullptr;
    }
}

void LVTimer::pause()
{
    if (m_timer) {
        lv_timer_pause(m_timer);
        ESP_LOGD(TAG, "Timer paused");
    }
}

void LVTimer::resume()
{
    if (m_timer) {
        lv_timer_resume(m_timer);
        ESP_LOGD(TAG, "Timer resumed");
    }
}

void LVTimer::reset()
{
    if (m_timer) {
        lv_timer_reset(m_timer);
        ESP_LOGD(TAG, "Timer reset");
    }
}

void LVTimer::setPeriod(uint32_t period_ms)
{
    if (m_timer) {
        lv_timer_set_period(m_timer, period_ms);
        ESP_LOGD(TAG, "Period set to %lu ms", period_ms);
    }
}

uint32_t LVTimer::getPeriod() const
{
    if (m_timer) {
        return m_timer->period;
    }
    return 0;
}

void LVTimer::setRepeatCount(int32_t count)
{
    if (m_timer) {
        lv_timer_set_repeat_count(m_timer, count);
        ESP_LOGD(TAG, "Repeat count set to %ld", count);
    }
}

int32_t LVTimer::getRepeatCount() const
{
    if (m_timer) {
        return m_timer->repeat_count;
    }
    return 0;
}

bool LVTimer::isPaused() const
{
    if (m_timer) {
        return m_timer->paused;
    }
    return true;
}

void LVTimer::ready()
{
    if (m_timer) {
        lv_timer_ready(m_timer);
    }
}

void LVTimer::timerCallback(lv_timer_t* timer)
{
    // Get the LVTimer instance from user_data
    LVTimer* instance = static_cast<LVTimer*>(timer->user_data);
    
    if (instance && instance->m_callback) {
        // Call the user's callback
        instance->m_callback();
    }
}
