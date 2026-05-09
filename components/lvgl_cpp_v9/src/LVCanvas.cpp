#include "LVCanvas.hpp"
#include <algorithm>

#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_rect.h"
#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_line.h"
#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_label.h"

LVCanvas::LVCanvas(lv_obj_t *parent, uint16_t width, uint16_t height, lv_color_format_t fmt, void *buffer)
    : m_width(width), m_height(height)
{
    m_canvas = lv_canvas_create(parent);
    setBuffer(buffer, width, height, fmt);
}

void LVCanvas::setBuffer(void *buffer, uint16_t width, uint16_t height, lv_color_format_t fmt)
{
    if (!m_canvas || !buffer)
        return;
    m_width = width;
    m_height = height;
    m_rawBuffer = buffer; // store for direct pixel writes
    lv_canvas_set_buffer(m_canvas, buffer, width, height, fmt);
}

void LVCanvas::fill(LVColor color, lv_opa_t opa)
{
    if (!m_canvas)
        return;
    lv_canvas_fill_bg(m_canvas, color.raw(), opa);
}

void LVCanvas::clear(LVColor color)
{
    fill(color, LV_OPA_COVER);
}

void LVCanvas::beginBatch()
{
    if (!m_canvas || m_batchMode)
        return;
    lv_canvas_init_layer(m_canvas, &m_batchLayer);
    m_batchMode = true;
}

void LVCanvas::endBatch()
{
    if (!m_canvas || !m_batchMode)
        return;
    m_batchMode = false;
    lv_canvas_finish_layer(m_canvas, &m_batchLayer);
}

lv_layer_t *LVCanvas::acquireLayer(lv_layer_t *tmp)
{
    if (m_batchMode)
        return &m_batchLayer;
    lv_canvas_init_layer(m_canvas, tmp);
    return tmp;
}

void LVCanvas::releaseLayer(lv_layer_t *tmp)
{
    if (!m_batchMode)
        lv_canvas_finish_layer(m_canvas, tmp);
}

void LVCanvas::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, LVColor color, lv_opa_t opa, int32_t radius)
{
    if (!m_canvas)
        return;
    lv_layer_t tmp;
    lv_layer_t *layer = acquireLayer(&tmp);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color.raw();
    dsc.bg_opa = opa;
    dsc.radius = radius;
    dsc.border_opa = LV_OPA_TRANSP;

    lv_area_t area;
    lv_area_set(&area, x, y, x + w - 1, y + h - 1);
    lv_draw_rect(layer, &dsc, &area);

    releaseLayer(&tmp);
}

void LVCanvas::drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, LVColor color, int32_t width, lv_opa_t opa)
{
    if (!m_canvas)
        return;
    lv_layer_t tmp;
    lv_layer_t *layer = acquireLayer(&tmp);

    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = color.raw();
    dsc.width = width;
    dsc.opa = opa;
    dsc.p1.x = x1;
    dsc.p1.y = y1;
    dsc.p2.x = x2;
    dsc.p2.y = y2;

    lv_draw_line(layer, &dsc);

    releaseLayer(&tmp);
}

void LVCanvas::drawCircle(int32_t cx, int32_t cy, int32_t radius, LVColor color, bool filled, lv_opa_t opa)
{
    if (!m_canvas || radius <= 0)
        return;

    // Use rounded rect with radius = diameter/2 to create circle
    int32_t diameter = radius * 2;
    int32_t x = cx - radius;
    int32_t y = cy - radius;

    lv_layer_t tmp;
    lv_layer_t *layer = acquireLayer(&tmp);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color.raw();
    dsc.bg_opa = filled ? opa : LV_OPA_TRANSP;
    dsc.radius = radius; // Full radius makes it circular

    if (!filled)
    {
        // Draw outline only
        dsc.border_color = color.raw();
        dsc.border_width = 2;
        dsc.border_opa = opa;
    }
    else
    {
        dsc.border_opa = LV_OPA_TRANSP;
    }

    lv_area_t area;
    lv_area_set(&area, x, y, x + diameter - 1, y + diameter - 1);
    lv_draw_rect(layer, &dsc, &area);

    releaseLayer(&tmp);
}

void LVCanvas::drawText(int32_t x, int32_t y, const char *text, LVColor color, int32_t fontSize, int32_t max_width)
{
    if (!m_canvas || !text)
        return;
    lv_layer_t tmp;
    lv_layer_t *layer = acquireLayer(&tmp);

    // Map fontSize to nearest available Montserrat built-in font
    const lv_font_t *font = &lv_font_montserrat_14; // fallback
    if (fontSize <= 15)
        font = &lv_font_montserrat_14;
    else if (fontSize <= 17)
        font = &lv_font_montserrat_16;
    else if (fontSize <= 19)
        font = &lv_font_montserrat_18;
    else if (fontSize <= 21)
        font = &lv_font_montserrat_20;
    else if (fontSize <= 23)
        font = &lv_font_montserrat_22;
    else if (fontSize <= 26)
        font = &lv_font_montserrat_24;
    else if (fontSize <= 30)
        font = &lv_font_montserrat_28;
    else if (fontSize <= 34)
        font = &lv_font_montserrat_32;
    else if (fontSize <= 42)
        font = &lv_font_montserrat_36;
    else
        font = &lv_font_montserrat_48;

    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = color.raw();
    dsc.font = font;
    dsc.text = text;
    int32_t w = (max_width > 0) ? max_width : lv_obj_get_width(m_canvas) - x;

    lv_area_t area;
    lv_area_set(&area, x, y, x + w - 1, y + fontSize * 2); // height = 2× fontSize to fit descenders
    lv_draw_label(layer, &dsc, &area);

    releaseLayer(&tmp);
}

void LVCanvas::setPalette(uint8_t idx, LVColor color)
{
    if (!m_canvas)
        return;
    lv_color32_t c32 = lv_color_to_32(color.raw(), LV_OPA_COVER);
    lv_canvas_set_palette(m_canvas, idx, c32);
}

void LVCanvas::fillHLine(int32_t x1, int32_t x2, int32_t y, LVColor color)
{
    if (!m_rawBuffer)
        return;
    if (y < 0 || y >= (int32_t)m_height)
        return;
    if (x1 > x2)
    {
        int32_t t = x1;
        x1 = x2;
        x2 = t;
    }
    if (x1 < 0)
        x1 = 0;
    if (x2 >= (int32_t)m_width)
        x2 = (int32_t)m_width - 1;
    if (x1 > x2)
        return;

    // Direct RGB565 write — std::fill lets compiler use SIMD/word writes
    uint16_t pixel = ((uint16_t)(color.r() >> 3) << 11) | ((uint16_t)(color.g() >> 2) << 5) | ((uint16_t)(color.b() >> 3));

    uint16_t *row = (uint16_t *)m_rawBuffer + y * m_width;
    std::fill(row + x1, row + x2 + 1, pixel);
}

void LVCanvas::invalidate()
{
    if (m_canvas)
        lv_obj_invalidate(m_canvas);
}
