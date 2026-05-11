#pragma once

#include "LVScreen.hpp"
#include "ScreenManager.hpp"
#include <cstdint>

/**
 * @brief LabBuddy HMI — Splash → Home → Library navigation
 *
 * Builds on the existing ScreenManager + LVScreen infrastructure.
 * Creates all screens, registers them, and starts the flow.
 *
 * Usage:
 * @code
 *   HMINavigator::getInstance().start();
 * @endcode
 */
class HMINavigator
{
public:
    static HMINavigator &getInstance();

    /**
     * @brief Create all screens, register with ScreenManager, show Splash.
     *        Must be called once after display init.
     */
    void start();

private:
    HMINavigator();
    ~HMINavigator();
    HMINavigator(const HMINavigator &) = delete;
    HMINavigator &operator=(const HMINavigator &) = delete;

    LVScreen *m_splash = nullptr;
    LVScreen *m_home = nullptr;
    LVScreen *m_library = nullptr;

    void buildSplashScreen(LVScreen *scr);
    void buildHomeScreen(LVScreen *scr);
    void buildLibraryScreen(LVScreen *scr);

    // Static LVGL event handlers
    static void onHomeBtnClicked(lv_event_t *e);
    static void onBackBtnClicked(lv_event_t *e);
    static void onLessonClicked(lv_event_t *e);

    // Splash auto-advance timer
    static void onSplashTimerCb(lv_timer_t *timer);
    lv_timer_t *m_splashTimer = nullptr;

    // ── Performance tracking ─────────────────────────────────────────
    struct PerfStats
    {
        uint32_t frame_count = 0;
        int64_t render_start_us = 0; ///< esp_timer timestamp at RENDER_START
        uint32_t render_time_us = 0; ///< last frame render time
        uint32_t render_sum_us = 0;  ///< accumulator for avg
        uint32_t render_min_us = UINT32_MAX;
        uint32_t render_max_us = 0;
        int64_t window_start_us = 0; ///< start of the 5-s log window
    };

    PerfStats m_perf;
    lv_timer_t *m_perfLogTimer = nullptr;

    static void onRenderStart(lv_event_t *e);
    static void onRenderReady(lv_event_t *e);
    static void onPerfLogTimerCb(lv_timer_t *timer);
};
