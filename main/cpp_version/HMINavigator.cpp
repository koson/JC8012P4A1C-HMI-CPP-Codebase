#include "HMINavigator.h"
#include "esp_log.h"

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
    delete m_splash;
    delete m_home;
    delete m_library;
}

// ── start() ───────────────────────────────────────────────────────────────────

void HMINavigator::start()
{
    ScreenManager &mgr = ScreenManager::getInstance();

    // Create screens
    m_splash  = new LVScreen();
    m_home    = new LVScreen();
    m_library = new LVScreen();

    // Populate content
    buildSplashScreen(m_splash);
    buildHomeScreen(m_home);
    buildLibraryScreen(m_library);

    // Register with existing ScreenManager
    mgr.registerScreen("splash",  m_splash);
    mgr.registerScreen("home",    m_home);
    mgr.registerScreen("library", m_library);

    ESP_LOGI(TAG, "Screens registered — loading Splash");
    mgr.navigateTo("splash", LVScreen::Transition::FadeIn, 400);

    // Auto-advance to Home after SPLASH_DURATION_MS
    m_splashTimer = lv_timer_create(onSplashTimerCb, SPLASH_DURATION_MS, this);
    lv_timer_set_repeat_count(m_splashTimer, 1);
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
        {LV_SYMBOL_LIST,     "คลังบทเรียน", 0x0f4c75, "library"},
        {LV_SYMBOL_PLAY,     "ทดลองวงจร",  0x1a5e2a, "library"},
        {LV_SYMBOL_EDIT,     "แบบฝึกหัด",  0x5e1a1a, "library"},
        {LV_SYMBOL_SETTINGS, "ตั้งค่า",    0x3a3a1a, "library"},
    };

    const int32_t BTN_W  = 270;
    const int32_t BTN_H  = 120;
    const int32_t GAP    = 20;
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

    static const char *LESSONS[] = {
        "L001 \xe2\x80\x94 NOT Gate",
        "L002 \xe2\x80\x94 AND Gate",
        "L003 \xe2\x80\x94 OR Gate",
        "L004 \xe2\x80\x94 NAND / NOR",
        "L005 \xe2\x80\x94 Half Adder",
        "(more lessons coming...)",
    };

    for (const char *lesson : LESSONS)
    {
        lv_obj_t *item = lv_obj_create(list);
        lv_obj_set_size(item, LV_PCT(100), 56);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x16213e), 0);
        lv_obj_set_style_radius(item, 8, 0);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *lbl = lv_label_create(item);
        lv_label_set_text(lbl, lesson);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xccccdd), 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 12, 0);
    }

    ESP_LOGI(TAG, "Library screen built");
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void HMINavigator::onSplashTimerCb(lv_timer_t *timer)
{
    HMINavigator *self = static_cast<HMINavigator *>(lv_timer_get_user_data(timer));
    self->m_splashTimer = nullptr;
    ESP_LOGI(TAG, "Splash timer — navigating to Home");
    ScreenManager::getInstance().navigateTo("home", LVScreen::Transition::FadeIn, 500);
}

void HMINavigator::onHomeBtnClicked(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target_obj(e);
    const char *target = static_cast<const char *>(lv_obj_get_user_data(btn));
    if (target)
        ScreenManager::getInstance().push(target, LVScreen::Transition::MoveLeft, 300);
}

void HMINavigator::onBackBtnClicked(lv_event_t *e)
{
    (void)e;
    ScreenManager::getInstance().back();
}
