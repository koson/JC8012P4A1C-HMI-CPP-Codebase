#pragma once

#include "lvgl.h"
#include "LVColor.hpp"
#include "SvgTypes.hpp"
#include <cstdint>
#include <vector>

/**
 * @brief Abstract drawing target used by renderers.
 *
 * This seam allows production rendering via LVCanvas and test rendering
 * via FakeDrawTarget without changing renderer logic.
 */
class DrawTarget
{
public:
    virtual ~DrawTarget() = default;

    virtual uint16_t width() const = 0;
    virtual uint16_t height() const = 0;

    virtual void fill(LVColor color, lv_opa_t opa = LV_OPA_COVER) = 0;
    virtual void beginBatch() = 0;
    virtual void endBatch() = 0;

    virtual void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, LVColor color, lv_opa_t opa = LV_OPA_COVER, int32_t radius = 0) = 0;
    virtual void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, LVColor color, int32_t width = 2, lv_opa_t opa = LV_OPA_COVER) = 0;
    virtual void drawCircle(int32_t cx, int32_t cy, int32_t radius, LVColor color, bool filled = true, lv_opa_t opa = LV_OPA_COVER) = 0;
    virtual void drawEllipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, LVColor fillColor, LVColor strokeColor, int32_t strokeWidth = 0, lv_opa_t opa = LV_OPA_COVER) = 0;
    virtual void drawText(int32_t x, int32_t y, const char *text, LVColor color, int32_t fontSize = 14, int32_t max_width = 0) = 0;

    virtual void fillHLine(int32_t x1, int32_t x2, int32_t y, LVColor color) = 0;
    virtual void invalidate() = 0;

    virtual bool hasVectorSupport() const { return false; }

    virtual void drawVectorPath(const std::vector<SvgRenderer::PathCommand> &commands,
                                int32_t offsetX, int32_t offsetY,
                                float scaleX, float scaleY,
                                LVColor fillColor, LVColor strokeColor,
                                int32_t strokeWidth = 0, lv_opa_t fillOpa = LV_OPA_COVER) {}
};
