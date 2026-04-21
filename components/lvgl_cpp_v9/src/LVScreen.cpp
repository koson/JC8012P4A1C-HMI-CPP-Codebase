#include "LVScreen.hpp"
#include "esp_log.h"

static const char* TAG = "LVScreen";

LVScreen::LVScreen(lv_obj_t* parent)
    : screen(nullptr)
    , load_cb(nullptr)
    , unload_cb(nullptr)
    , owns_screen(true)
{
    if (parent) {
        screen = parent;
        owns_screen = false;
    } else {
        screen = lv_obj_create(nullptr);
        if (!screen) {
            ESP_LOGE(TAG, "Failed to create screen");
            return;
        }
    }
    
    // Set default screen properties
    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));
}

LVScreen::~LVScreen()
{
    // Only delete if we own the screen and it's not currently active
    if (owns_screen && screen && !isActive()) {
        lv_obj_delete(screen);
    }
}

void LVScreen::load(Transition transition, uint32_t duration)
{
    if (!screen) {
        ESP_LOGE(TAG, "Cannot load null screen");
        return;
    }
    
    // Call load callback before loading
    if (load_cb) {
        load_cb();
    }
    
    // Load screen with transition
    lv_screen_load_anim_t anim_type = getTransitionType(transition);
    
    if (transition == Transition::None || duration == 0) {
        lv_screen_load(screen);
    } else {
        lv_screen_load_anim(screen, anim_type, duration, 0, false);
    }
    
    ESP_LOGI(TAG, "Screen loaded with transition=%d, duration=%lu ms", 
             static_cast<int>(transition), duration);
}

bool LVScreen::isActive() const
{
    if (!screen) return false;
    return lv_screen_active() == screen;
}

LVScreen& LVScreen::onLoad(std::function<void()> callback)
{
    load_cb = callback;
    return *this;
}

LVScreen& LVScreen::onUnload(std::function<void()> callback)
{
    unload_cb = callback;
    return *this;
}

LVScreen& LVScreen::setBackgroundColor(lv_color_t color)
{
    if (screen) {
        lv_obj_set_style_bg_color(screen, color, 0);
    }
    return *this;
}

LVScreen& LVScreen::setBackgroundOpacity(lv_opa_t opa)
{
    if (screen) {
        lv_obj_set_style_bg_opa(screen, opa, 0);
    }
    return *this;
}

LVScreen& LVScreen::setScrollable(bool enable)
{
    if (screen) {
        if (enable) {
            lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        } else {
            lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        }
    }
    return *this;
}

void LVScreen::clean()
{
    if (screen) {
        lv_obj_clean(screen);
        ESP_LOGI(TAG, "Screen cleaned");
    }
}

lv_screen_load_anim_t LVScreen::getTransitionType(Transition transition)
{
    switch (transition) {
        case Transition::None:
            return LV_SCR_LOAD_ANIM_NONE;
        case Transition::FadeIn:
            return LV_SCR_LOAD_ANIM_FADE_IN;
        case Transition::FadeOut:
            return LV_SCR_LOAD_ANIM_FADE_OUT;
        case Transition::MoveLeft:
            return LV_SCR_LOAD_ANIM_MOVE_LEFT;
        case Transition::MoveRight:
            return LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        case Transition::MoveTop:
            return LV_SCR_LOAD_ANIM_MOVE_TOP;
        case Transition::MoveBottom:
            return LV_SCR_LOAD_ANIM_MOVE_BOTTOM;
        case Transition::OverLeft:
            return LV_SCR_LOAD_ANIM_OVER_LEFT;
        case Transition::OverRight:
            return LV_SCR_LOAD_ANIM_OVER_RIGHT;
        case Transition::OverTop:
            return LV_SCR_LOAD_ANIM_OVER_TOP;
        case Transition::OverBottom:
            return LV_SCR_LOAD_ANIM_OVER_BOTTOM;
        default:
            return LV_SCR_LOAD_ANIM_NONE;
    }
}
