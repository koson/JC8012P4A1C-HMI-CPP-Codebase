#pragma once

#include "lvgl.h"
#include <cstdint>

/**
 * @brief Display wrapper for LVGL v9 (default display)
 */
class LVDisplay {
public:
    enum class Rotation {
        Rotate0,
        Rotate90,
        Rotate180,
        Rotate270,
    };

    /**
     * @brief Get wrapper for default display
     */
    static LVDisplay& getDefault();

    /**
     * @brief Get raw LVGL display pointer
     */
    lv_display_t* raw() const { return m_disp; }

    /**
     * @brief Set display rotation
     */
    void setRotation(Rotation rot);

    /**
     * @brief Get current rotation
     */
    Rotation getRotation() const;

    /**
     * @brief Set resolution (hor, ver)
     */
    void setResolution(uint32_t hor, uint32_t ver);

    /**
     * @brief Get horizontal resolution
     */
    uint32_t horRes() const;

    /**
     * @brief Get vertical resolution
     */
    uint32_t verRes() const;

    /**
     * @brief Get display DPI
     */
    uint32_t dpi() const;

    /**
     * @brief Load a screen with simple fade animation
     * @param screen screen object to load
     * @param anim_time_ms fade duration (default 300ms)
     */
    void loadScreen(lv_obj_t* screen, uint32_t anim_time_ms = 300);

    /**
     * @brief Load a screen immediately (no animation)
     */
    void setActiveScreen(lv_obj_t* screen);

private:
    explicit LVDisplay(lv_display_t* disp);
    lv_display_t* m_disp;
};
