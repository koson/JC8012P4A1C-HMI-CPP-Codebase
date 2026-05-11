#include "HMINavigator.h"
#include "LessonPlayer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "src/debugging/sysmon/lv_sysmon.h"
#include "esp_lvgl_port.h"

static const char *TAG = "HMINavigator";

static constexpr uint32_t SPLASH_DURATION_MS = 2000;

// ── Singleton ─────────────────────────────────────────────────────────────────

HMINavigator &HMINavigator::getInstance()
{
    static HMINavigator instance;
    return instance;
}

HMINavigator::HMINavigator() {}

HMINavigator::~HMINavigator()
{
    if (m_splashTimer)
    {
        lv_timer_delete(m_splashTimer);
        m_splashTimer = nullptr;
    }
    if (m_perfLogTimer)
    {
        lv_timer_delete(m_perfLogTimer);
        m_perfLogTimer = nullptr;
    }
    delete m_splash;
    delete m_home;
    delete m_library;
}

// ── start() ───────────────────────────────────────────────────────────────────

void HMINavigator::start()
{
    // All LVGL API calls from non-LVGL tasks require the port mutex.
    // Without it, the LVGL render task races with object creation → style NULL-deref crash.
    lvgl_port_lock(0);

    ScreenManager &mgr = ScreenManager::getInstance();

    // Create screens
    m_splash = new LVScreen();
    m_home = new LVScreen();
    m_library = new LVScreen();

    // Populate content
    buildSplashScreen(m_splash);
    buildHomeScreen(m_home);
    buildLibraryScreen(m_library);

    // Register with existing ScreenManager
    mgr.registerScreen("splash", m_splash);
    mgr.registerScreen("home", m_home);
    mgr.registerScreen("library", m_library);

    ESP_LOGI(TAG, "Screens registered — loading Splash");
    mgr.navigateTo("splash", LVScreen::Transition::FadeIn, 400);

    // ── Display event hooks for render-time tracking ──────────────────
    lv_display_t *disp = lv_display_get_default();
    if (disp)
    {
        lv_display_add_event_cb(disp, onRenderStart, LV_EVENT_RENDER_START, this);
        lv_display_add_event_cb(disp, onRenderReady, LV_EVENT_RENDER_READY, this);
    }

    // Init perf window
    m_perf = PerfStats{};
    m_perf.window_start_us = esp_timer_get_time();

    // Serial log every 5 s
    m_perfLogTimer = lv_timer_create(onPerfLogTimerCb, 5000, this);

    // Defer sysmon overlay: must run from within the LVGL task context,
    // not from app_main, otherwise lv_inv_area spins and WDT triggers.
    lv_timer_t *t = lv_timer_create(
        [](lv_timer_t *tmr)
        {
            lv_sysmon_show_performance(nullptr);
            ESP_LOGI("HMINavigator", "Performance overlay enabled");
            lv_timer_delete(tmr);
        },
        200, nullptr);
    lv_timer_set_repeat_count(t, 1);

    // Auto-advance to Home after SPLASH_DURATION_MS
    m_splashTimer = lv_timer_create(onSplashTimerCb, SPLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(m_splashTimer, 1);

    lvgl_port_unlock();
}

// ── Splash Screen ─────────────────────────────────────────────────────────────

void HMINavigator::buildSplashScreen(LVScreen *scr)
{
    lv_obj_t *root = scr->getObj();
    lv_obj_set_style_bg_color(root, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    lv_obj_t *logo = lv_label_create(root);
    lv_label_set_text(logo, "LabBuddy");
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(logo, lv_color_hex(0x00d4ff), 0);
    lv_obj_center(logo);

    lv_obj_t *tag = lv_label_create(root);
    lv_label_set_text(tag, "Digital Logic Learning Lab");
    lv_obj_set_style_text_font(tag, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(tag, lv_color_hex(0x8888aa), 0);
    lv_obj_align_to(tag, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    lv_obj_t *ver = lv_label_create(root);
    lv_label_set_text(ver, "v0.2 — Sprint 2");
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(ver, lv_color_hex(0x555577), 0);
    lv_obj_align(ver, LV_ALIGN_BOTTOM_MID, 0, -20);

    ESP_LOGI(TAG, "Splash screen built");
}

// ── Home Screen ───────────────────────────────────────────────────────────────

void HMINavigator::buildHomeScreen(LVScreen *scr)
{
    lv_obj_t *root = scr->getObj();
    lv_obj_set_style_bg_color(root, lv_color_hex(0x0f0f23), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Status bar
    lv_obj_t *bar = lv_obj_create(root);
    lv_obj_set_size(bar, LV_HOR_RES, 40);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_wifi = lv_label_create(bar);
    lv_label_set_text(lbl_wifi, LV_SYMBOL_WIFI "  LabBuddy");
    lv_obj_set_style_text_color(lbl_wifi, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_text_font(lbl_wifi, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_wifi, LV_ALIGN_LEFT_MID, 12, 0);

    // Title
    lv_obj_t *title = lv_label_create(root);
    lv_label_set_text(title, "LabBuddy");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00d4ff), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 56);

    lv_obj_t *sub = lv_label_create(root);
    lv_label_set_text(sub, "เลือกหมวดบทเรียน");
    lv_obj_set_style_text_color(sub, lv_color_hex(0x8888aa), 0);
    lv_obj_align_to(sub, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    // Category buttons — 2×2 grid
    struct BtnDef
    {
        const char *icon;
        const char *label;
        uint32_t color;
        const char *target; // ScreenManager name
    };

    static const BtnDef BTNS[] = {
        {LV_SYMBOL_LIST, "คลังบทเรียน", 0x0f4c75, "library"},
        {LV_SYMBOL_PLAY, "ทดลองวงจร", 0x1a5e2a, "library"},
        {LV_SYMBOL_EDIT, "แบบฝึกหัด", 0x5e1a1a, "library"},
        {LV_SYMBOL_SETTINGS, "ตั้งค่า", 0x3a3a1a, "library"},
    };

    const int32_t BTN_W = 270;
    const int32_t BTN_H = 120;
    const int32_t GAP = 20;
    const int32_t START_Y = 150;
    const int32_t START_X = (LV_HOR_RES - (BTN_W * 2 + GAP)) / 2;

    for (int i = 0; i < 4; i++)
    {
        int col = i % 2;
        int row = i / 2;

        lv_obj_t *btn = lv_obj_create(root);
        lv_obj_set_size(btn, BTN_W, BTN_H);
        lv_obj_set_pos(btn, START_X + col * (BTN_W + GAP),
                       START_Y + row * (BTN_H + GAP));
        lv_obj_set_style_bg_color(btn, lv_color_hex(BTNS[i].color), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x3a3a5a), 0);
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(btn,
                                  lv_color_lighten(lv_color_hex(BTNS[i].color), 40), LV_STATE_PRESSED);

        lv_obj_t *icon = lv_label_create(btn);
        lv_label_set_text(icon, BTNS[i].icon);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(icon, lv_color_hex(0xffffff), 0);
        lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 12);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, BTNS[i].label);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xddddee), 0);
        lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -12);

        // Store target screen name pointer in user_data
        lv_obj_set_user_data(btn, (void *)BTNS[i].target);
        lv_obj_add_event_cb(btn, onHomeBtnClicked, LV_EVENT_CLICKED, nullptr);
    }

    ESP_LOGI(TAG, "Home screen built");
}

// ── Library Screen ────────────────────────────────────────────────────────────

void HMINavigator::buildLibraryScreen(LVScreen *scr)
{
    lv_obj_t *root = scr->getObj();
    lv_obj_set_style_bg_color(root, lv_color_hex(0x0f0f23), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Header
    lv_obj_t *header = lv_obj_create(root);
    lv_obj_set_size(header, LV_HOR_RES, 60);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *btn_back = lv_btn_create(header);
    lv_obj_set_size(btn_back, 80, 40);
    lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_add_event_cb(btn_back, onBackBtnClicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *back_lbl = lv_label_create(btn_back);
    lv_label_set_text(back_lbl, LV_SYMBOL_LEFT " Back");
    lv_obj_center(back_lbl);

    lv_obj_t *hdr_title = lv_label_create(header);
    lv_label_set_text(hdr_title, "คลังบทเรียน");
    lv_obj_set_style_text_font(hdr_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(hdr_title, lv_color_hex(0x00d4ff), 0);
    lv_obj_align(hdr_title, LV_ALIGN_CENTER, 0, 0);

    // Lesson list
    lv_obj_t *list = lv_obj_create(root);
    lv_obj_set_size(list, LV_HOR_RES - 40, LV_VER_RES - 80);
    lv_obj_set_pos(list, 20, 70);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_style_pad_all(list, 12, 0);

    struct LessonEntry
    {
        const char *display;   // shown in the list
        const char *json_path; // SD card path (nullptr = coming soon)
        bool available;
    };
    static const LessonEntry LESSONS[] = {
        {"L001 \xe2\x80\x94 NOT Gate", "/sdcard/lessons/L001_not_gate.json", true},
        {"L002 \xe2\x80\x94 AND Gate", nullptr, false},
        {"L003 \xe2\x80\x94 OR Gate", nullptr, false},
        {"L004 \xe2\x80\x94 NAND / NOR", nullptr, false},
        {"L005 \xe2\x80\x94 Half Adder", nullptr, false},
    };

    for (const auto &lesson : LESSONS)
    {
        lv_obj_t *item = lv_obj_create(list);
        lv_obj_set_size(item, LV_PCT(100), 64);
        lv_obj_set_style_bg_color(item,
                                  lesson.available ? lv_color_hex(0x16213e) : lv_color_hex(0x1a1a2e), 0);
        lv_obj_set_style_radius(item, 8, 0);
        lv_obj_set_style_border_color(item,
                                      lesson.available ? lv_color_hex(0x3a3a5a) : lv_color_hex(0x2a2a3a), 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

        if (lesson.available)
        {
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
            // Store SD card path in user_data
            lv_obj_set_user_data(item, (void *)lesson.json_path);
            lv_obj_add_event_cb(item, onLessonClicked, LV_EVENT_CLICKED, nullptr);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x1e2d4a),
                                      LV_STATE_PRESSED);
        }

        // Arrow indicator for available lessons
        if (lesson.available)
        {
            lv_obj_t *arrow = lv_label_create(item);
            lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
            lv_obj_set_style_text_color(arrow, lv_color_hex(0x00d4ff), 0);
            lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -16, 0);
        }

        lv_obj_t *lbl = lv_label_create(item);
        lv_label_set_text(lbl, lesson.display);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(lbl,
                                    lesson.available ? lv_color_hex(0xffffff) : lv_color_hex(0x556677), 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 16, 0);
    }

    ESP_LOGI(TAG, "Library screen built");
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void HMINavigator::onSplashTimerCb(lv_timer_t *timer)
{
    HMINavigator *self = static_cast<HMINavigator *>(lv_timer_get_user_data(timer));
    self->m_splashTimer = nullptr;
    ESP_LOGI(TAG, "Splash timer — navigating to Home");
    ScreenManager::getInstance().navigateTo("home", LVScreen::Transition::FadeIn, 400);
}

void HMINavigator::onHomeBtnClicked(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target_obj(e);
    const char *target = static_cast<const char *>(lv_obj_get_user_data(btn));
    if (target)
        ScreenManager::getInstance().push(target, LVScreen::Transition::OverLeft, 200);
}

void HMINavigator::onBackBtnClicked(lv_event_t *e)
{
    (void)e;
    ScreenManager::getInstance().back(LVScreen::Transition::OverRight, 200);
}

void HMINavigator::onLessonClicked(lv_event_t *e)
{
    lv_obj_t *item = lv_event_get_target_obj(e);
    const char *json_path = static_cast<const char *>(lv_obj_get_user_data(item));
    if (!json_path)
        return;

    // Must release LVGL lock before LessonPlayer::loadLesson (file I/O)
    // then re-acquire inside LessonPlayer::show()
    lvgl_port_unlock();

    LessonPlayer &lp = LessonPlayer::getInstance();
    if (lp.loadLesson(json_path))
        lp.show();
    else
    {
        lvgl_port_lock(0);
        // Show toast-style error
        lv_obj_t *toast = lv_label_create(lv_scr_act());
        lv_label_set_text_fmt(toast, "Cannot load: %s", json_path);
        lv_obj_set_style_text_color(toast, lv_color_hex(0xff4444), 0);
        lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -20);
        lvgl_port_unlock();
    }
}

// ── Performance callbacks ─────────────────────────────────────────────────────

void HMINavigator::onRenderStart(lv_event_t *e)
{
    HMINavigator *self = static_cast<HMINavigator *>(lv_event_get_user_data(e));
    self->m_perf.render_start_us = esp_timer_get_time();
}

void HMINavigator::onRenderReady(lv_event_t *e)
{
    HMINavigator *self = static_cast<HMINavigator *>(lv_event_get_user_data(e));
    PerfStats &p = self->m_perf;

    if (p.render_start_us == 0)
        return;

    uint32_t elapsed = (uint32_t)(esp_timer_get_time() - p.render_start_us);
    p.render_time_us = elapsed;
    p.render_sum_us += elapsed;
    if (elapsed < p.render_min_us)
        p.render_min_us = elapsed;
    if (elapsed > p.render_max_us)
        p.render_max_us = elapsed;
    p.frame_count++;
}

void HMINavigator::onPerfLogTimerCb(lv_timer_t *timer)
{
    HMINavigator *self = static_cast<HMINavigator *>(lv_timer_get_user_data(timer));
    PerfStats &p = self->m_perf;

    int64_t now = esp_timer_get_time();
    int64_t window_us = now - p.window_start_us;

    if (p.frame_count == 0 || window_us <= 0)
    {
        ESP_LOGI(TAG, "[PERF] No frames rendered in last 5 s");
        return;
    }

    float fps = (float)p.frame_count * 1e6f / (float)window_us;
    float avg_ms = (float)p.render_sum_us / p.frame_count / 1000.0f;
    float min_ms = (float)p.render_min_us / 1000.0f;
    float max_ms = (float)p.render_max_us / 1000.0f;
    float last_ms = (float)p.render_time_us / 1000.0f;

    ESP_LOGI(TAG, "[PERF] FPS: %.1f  frames: %lu  render: avg=%.2f min=%.2f max=%.2f last=%.2f (ms)",
             fps, (unsigned long)p.frame_count, avg_ms, min_ms, max_ms, last_ms);

    // Reset window
    p.frame_count = 0;
    p.render_sum_us = 0;
    p.render_min_us = UINT32_MAX;
    p.render_max_us = 0;
    p.window_start_us = now;
}
