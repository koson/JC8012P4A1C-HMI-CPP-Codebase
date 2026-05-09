#pragma once

#include "lvgl.h"
#include "LVColor.hpp"
#include <cstdint>

/**
 * @brief Canvas wrapper for LVGL v9 (RGB565/ARGB8888/etc.).
 * Non-owning buffer (caller manages memory lifetime).
 *
 * Use beginBatch()/endBatch() to group all draw calls into a single
 * lv_canvas_finish_layer flush instead of one flush per primitive.
 */
class LVCanvas
{
public:
    LVCanvas(lv_obj_t *parent, uint16_t width, uint16_t height, lv_color_format_t fmt, void *buffer);

    lv_obj_t *obj() const { return m_canvas; }

    uint16_t width() const { return m_width; }
    uint16_t height() const { return m_height; }

    void setBuffer(void *buffer, uint16_t width, uint16_t height, lv_color_format_t fmt);

    void fill(LVColor color, lv_opa_t opa = LV_OPA_COVER);
    void clear(LVColor color = LVColor::Black);

    /** Open a shared layer – all draw calls until endBatch() reuse it. */
    void beginBatch();
    /** Flush the shared layer to the display once. */
    void endBatch();

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, LVColor color, lv_opa_t opa = LV_OPA_COVER, int32_t radius = 0);
    void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, LVColor color, int32_t width = 2, lv_opa_t opa = LV_OPA_COVER);
    void drawCircle(int32_t cx, int32_t cy, int32_t radius, LVColor color, bool filled = true, lv_opa_t opa = LV_OPA_COVER);
    void drawText(int32_t x, int32_t y, const char *text, LVColor color, int32_t fontSize = 14, int32_t max_width = 0);

    /** Direct RGB565 pixel write — bypasses LVGL draw tasks, much faster for fills. */
    void fillHLine(int32_t x1, int32_t x2, int32_t y, LVColor color);
    /** Tell LVGL the canvas buffer has changed and needs redisplay. */
    void invalidate();

    void setPalette(uint8_t idx, LVColor color); // for indexed formats

private:
    lv_obj_t *m_canvas;
    uint16_t m_width;
    uint16_t m_height;

    bool m_batchMode = false;
    lv_layer_t m_batchLayer;
    void *m_rawBuffer = nullptr; // Raw pixel buffer pointer for direct writes (RGB565)

    /** Get active layer: batch layer if open, else init a temporary one. */
    lv_layer_t *acquireLayer(lv_layer_t *tmp);
    /** Finish the layer only when NOT in batch mode. */
    void releaseLayer(lv_layer_t *tmp);
};
