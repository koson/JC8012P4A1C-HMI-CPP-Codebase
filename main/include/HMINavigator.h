#pragma once

#include "LVScreen.hpp"
#include "ScreenManager.hpp"

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

    // Splash auto-advance timer
    static void onSplashTimerCb(lv_timer_t *timer);
    lv_timer_t *m_splashTimer = nullptr;
};
