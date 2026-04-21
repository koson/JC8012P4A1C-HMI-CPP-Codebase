#include "LVColor.hpp"

#include <cstdio>

LVColor::LVColor() : m_color(lv_color_black()) {}
LVColor::LVColor(uint8_t r, uint8_t g, uint8_t b) : m_color(lv_color_make(r, g, b)) {}
LVColor::LVColor(uint32_t hex_rgb) : m_color(lv_color_hex(hex_rgb & 0x00FFFFFF)) {}

LVColor LVColor::fromHSV(uint16_t h_deg, uint8_t s_percent, uint8_t v_percent)
{
    return LVColor(lv_color_hsv_to_rgb(h_deg, s_percent, v_percent));
}

uint8_t LVColor::r() const
{
    return m_color.red;
}

uint8_t LVColor::g() const
{
    return m_color.green;
}

uint8_t LVColor::b() const
{
    return m_color.blue;
}

uint32_t LVColor::toHex() const
{
    return lv_color_to_u32(m_color) & 0x00FFFFFFu;
}

std::string LVColor::toHexString() const
{
    char buf[10];
    std::snprintf(buf, sizeof(buf), "#%06X", static_cast<unsigned>(toHex()));
    return std::string(buf);
}

LVColor LVColor::lighten(uint8_t amount) const
{
    return LVColor(lv_color_lighten(m_color, amount));
}

LVColor LVColor::darken(uint8_t amount) const
{
    return LVColor(lv_color_darken(m_color, amount));
}

LVColor LVColor::mix(const LVColor& other, uint8_t ratio) const
{
    return LVColor(lv_color_mix(m_color, other.m_color, ratio));
}

// Predefined colors
const LVColor LVColor::White   = LVColor(0xFFFFFF);
const LVColor LVColor::Black   = LVColor(0x000000);
const LVColor LVColor::Red     = LVColor(0xFF0000);
const LVColor LVColor::Green   = LVColor(0x00FF00);
const LVColor LVColor::Blue    = LVColor(0x0000FF);
const LVColor LVColor::Cyan    = LVColor(0x00FFFF);
const LVColor LVColor::Magenta = LVColor(0xFF00FF);
const LVColor LVColor::Yellow  = LVColor(0xFFFF00);
const LVColor LVColor::Gray    = LVColor(0x808080);
