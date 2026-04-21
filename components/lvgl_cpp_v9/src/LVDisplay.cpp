#include "LVDisplay.hpp"

#include "esp_log.h"

static const char* TAG = "LVDisplay";

LVDisplay::LVDisplay(lv_display_t* disp) : m_disp(disp) {}

LVDisplay& LVDisplay::getDefault()
{
    static LVDisplay instance(lv_display_get_default());
    return instance;
}

void LVDisplay::setRotation(Rotation rot)
{
    if (!m_disp) return;
    lv_display_rotation_t lvrot = LV_DISPLAY_ROTATION_0;
    switch (rot) {
        case Rotation::Rotate0: lvrot = LV_DISPLAY_ROTATION_0; break;
        case Rotation::Rotate90: lvrot = LV_DISPLAY_ROTATION_90; break;
        case Rotation::Rotate180: lvrot = LV_DISPLAY_ROTATION_180; break;
        case Rotation::Rotate270: lvrot = LV_DISPLAY_ROTATION_270; break;
    }
    lv_display_set_rotation(m_disp, lvrot);
}

LVDisplay::Rotation LVDisplay::getRotation() const
{
    if (!m_disp) return Rotation::Rotate0;
    lv_display_rotation_t lvrot = lv_display_get_rotation(m_disp);
    switch (lvrot) {
        case LV_DISPLAY_ROTATION_90: return Rotation::Rotate90;
        case LV_DISPLAY_ROTATION_180: return Rotation::Rotate180;
        case LV_DISPLAY_ROTATION_270: return Rotation::Rotate270;
        case LV_DISPLAY_ROTATION_0:
        default:
            return Rotation::Rotate0;
    }
}

void LVDisplay::setResolution(uint32_t hor, uint32_t ver)
{
    if (!m_disp) return;
    lv_display_set_resolution(m_disp, hor, ver);
}

uint32_t LVDisplay::horRes() const
{
    return m_disp ? lv_display_get_horizontal_resolution(m_disp) : 0;
}

uint32_t LVDisplay::verRes() const
{
    return m_disp ? lv_display_get_vertical_resolution(m_disp) : 0;
}

uint32_t LVDisplay::dpi() const
{
    return m_disp ? lv_display_get_dpi(m_disp) : 0;
}

void LVDisplay::loadScreen(lv_obj_t* screen, uint32_t anim_time_ms)
{
    if (!screen) return;
    // Keep previous screen alive to allow toggling back and forth
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, anim_time_ms, 0, false);
}

void LVDisplay::setActiveScreen(lv_obj_t* screen)
{
    if (!screen) return;
    lv_scr_load(screen);
}
