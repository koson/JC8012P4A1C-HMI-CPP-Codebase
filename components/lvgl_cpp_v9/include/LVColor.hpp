#pragma once

#include "lvgl.h"
#include <cstdint>
#include <string>

/**
 * @brief LVGL color wrapper with RGB/HSV helpers and basic ops.
 */
class LVColor {
public:
    LVColor();
    LVColor(uint8_t r, uint8_t g, uint8_t b);
    explicit LVColor(uint32_t hex_rgb);

    static LVColor fromHSV(uint16_t h_deg, uint8_t s_percent, uint8_t v_percent);
    static LVColor fromRaw(lv_color_t c) { return LVColor(c); }

    uint8_t r() const;
    uint8_t g() const;
    uint8_t b() const;

    uint32_t toHex() const;          // 0xRRGGBB
    std::string toHexString() const; // "#RRGGBB"

    LVColor lighten(uint8_t amount) const; // amount 0-255
    LVColor darken(uint8_t amount) const;  // amount 0-255
    LVColor mix(const LVColor& other, uint8_t ratio) const; // ratio 0-255, 255 = this

    lv_color_t raw() const { return m_color; }

    // Predefined colors
    static const LVColor White;
    static const LVColor Black;
    static const LVColor Red;
    static const LVColor Green;
    static const LVColor Blue;
    static const LVColor Cyan;
    static const LVColor Magenta;
    static const LVColor Yellow;
    static const LVColor Gray;

private:
    explicit LVColor(lv_color_t c) : m_color(c) {}
    lv_color_t m_color;
};
