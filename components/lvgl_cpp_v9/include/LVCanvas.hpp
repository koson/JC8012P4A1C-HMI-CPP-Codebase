#pragma once

#include "lvgl.h"
#include "LVColor.hpp"
#include <cstdint>

/**
 * @brief Canvas wrapper for LVGL v9 (RGB565/ARGB8888/etc.).
 * Non-owning buffer (caller manages memory lifetime).
 */
class LVCanvas {
public:
    LVCanvas(lv_obj_t* parent, uint16_t width, uint16_t height, lv_color_format_t fmt, void* buffer);

    lv_obj_t* obj() const { return m_canvas; }

    void setBuffer(void* buffer, uint16_t width, uint16_t height, lv_color_format_t fmt);

    void fill(LVColor color, lv_opa_t opa = LV_OPA_COVER);
    void clear(LVColor color = LVColor::Black);

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, LVColor color, lv_opa_t opa = LV_OPA_COVER, int32_t radius = 0);
    void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, LVColor color, int32_t width = 2, lv_opa_t opa = LV_OPA_COVER);
    void drawText(int32_t x, int32_t y, const char* text, LVColor color, int32_t max_width = 0);

    void setPalette(uint8_t idx, LVColor color); // for indexed formats

private:
    lv_obj_t* m_canvas;
};
