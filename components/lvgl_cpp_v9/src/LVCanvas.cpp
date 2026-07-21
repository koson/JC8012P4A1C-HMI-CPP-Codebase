#include "LVCanvas.hpp"
#include <algorithm>
#include <cmath>

#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_rect.h"
#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_line.h"
#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_label.h"
#include "font_thai.h"

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
    m_textArena.clear(); // free Thai-shaped text buffers after all tasks have rendered
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

void LVCanvas::drawEllipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, LVColor fillColor, LVColor strokeColor, int32_t strokeWidth, lv_opa_t opa)
{
    if (!m_rawBuffer || rx <= 0 || ry <= 0)
        return;

    // True ellipse via scanlines: for each row y, compute x span from ellipse equation.
    // (dx/rx)^2 + (dy/ry)^2 = 1  →  dx = rx * sqrt(1 - (dy/ry)^2)
    //
    // Two-pass approach for stroke:
    //   1. Fill outer ellipse (rx, ry) with strokeColor
    //   2. Fill inner ellipse (rx-sw, ry-sw) with fillColor

    auto fillEllipse = [&](int32_t erx, int32_t ery, LVColor color)
    {
        if (erx <= 0 || ery <= 0)
            return;
        float erxf = (float)erx;
        float eryf = (float)ery;
        for (int32_t dy = -ery; dy <= ery; dy++)
        {
            float t = (float)dy / eryf;
            float dx = erxf * sqrtf(1.0f - t * t);
            fillHLine(cx - (int32_t)dx, cx + (int32_t)dx, cy + dy, color);
        }
    };

    if (strokeWidth > 0)
    {
        fillEllipse(rx, ry, strokeColor);
        fillEllipse(rx - strokeWidth, ry - strokeWidth, fillColor);
    }
    else
    {
        fillEllipse(rx, ry, fillColor);
    }
}

void LVCanvas::drawText(int32_t x, int32_t y, const char *text, LVColor color, int32_t fontSize, int32_t max_width)
{
    if (!m_canvas || !text)
        return;
    lv_layer_t tmp;
    lv_layer_t *layer = acquireLayer(&tmp);

    // Select font: always TH Niramit for consistent sizing between Thai and Latin-only text.
    // Montserrat renders Latin at a visually larger size than TH Niramit at the same fontSize,
    // which confuses users when mixing screens. TH Niramit contains full Latin (U+0020-007F).
    const lv_font_t *font = th_niramit_select(fontSize);

    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = color.raw();
    dsc.font = font;
    dsc.text = text;
    int32_t w = (max_width > 0) ? max_width : (int32_t)m_width - x;

    lv_area_t area;
    lv_area_set(&area, x, y, x + w - 1, y + fontSize * 2); // height = 2× fontSize to fit descenders

    // Always use the Thai shaper path — handles Latin-only passthrough correctly
    // (no tone marks extracted → base_buf == original text). This also ensures
    // Latin characters use TH Niramit sizing for consistent appearance.
    drawTextThaiShaped(layer, &dsc, &area, fontSize);

    releaseLayer(&tmp);
    if (!m_batchMode)
        m_textArena.clear(); // free Thai-shaped text buffers after non-batch finish_layer
}

// ─── Thai tone mark shaper ────────────────────────────────────────────────────
// Thai character classification helpers (static, file-scope)
static inline bool th_is_above_vowel(uint32_t cp)
{
    // ั(0E31)  ิ(0E34) ี(0E35) ึ(0E36) ื(0E37)  ็(0E47)  ํ(0E4D)
    return (cp == 0x0E31) ||
           (cp >= 0x0E34 && cp <= 0x0E37) ||
           (cp == 0x0E47) ||
           (cp == 0x0E4D);
}

static inline bool th_is_tone_mark(uint32_t cp)
{
    // ่(0E48) ้(0E49) ๊(0E4A) ๋(0E4B)
    return (cp >= 0x0E48 && cp <= 0x0E4B);
}

// Read one UTF-8 codepoint from *p, advance *p. Returns 0 at end of string.
static uint32_t utf8_next_cp(const uint8_t **p)
{
    uint8_t b = **p;
    if (b == 0)
        return 0;
    (*p)++;
    if (b < 0x80)
        return b;
    if ((b & 0xE0) == 0xC0)
    {
        uint32_t cp = (uint32_t)(b & 0x1F) << 6;
        cp |= (**p & 0x3F);
        (*p)++;
        return cp;
    }
    if ((b & 0xF0) == 0xE0)
    {
        uint32_t cp = (uint32_t)(b & 0x0F) << 12;
        cp |= (uint32_t)(**p & 0x3F) << 6;
        (*p)++;
        cp |= (**p & 0x3F);
        (*p)++;
        return cp;
    }
    // 4-byte (outside BMP, skip)
    (*p) += 3;
    return 0xFFFD;
}

// Write one UTF-8 codepoint to buf (must have at least 4 bytes). Returns byte count.
static int utf8_write_cp(uint8_t *buf, uint32_t cp)
{
    if (cp < 0x80)
    {
        buf[0] = (uint8_t)cp;
        return 1;
    }
    if (cp < 0x800)
    {
        buf[0] = 0xC0 | (uint8_t)(cp >> 6);
        buf[1] = 0x80 | (uint8_t)(cp & 0x3F);
        return 2;
    }
    buf[0] = 0xE0 | (uint8_t)(cp >> 12);
    buf[1] = 0x80 | (uint8_t)((cp >> 6) & 0x3F);
    buf[2] = 0x80 | (uint8_t)(cp & 0x3F);
    return 3;
}

void LVCanvas::drawTextThaiShaped(lv_layer_t *layer, const lv_draw_label_dsc_t *dsc,
                                  const lv_area_t *area, int32_t fontSize)
{
    struct ToneMark
    {
        int32_t x;        // screen x1 for the separate tone-mark draw call
        uint32_t cp;      // tone mark codepoint (่ ้ ๊ ๋)
        uint32_t base_cp; // base consonant codepoint — used to compute dynamic offset
    };

    static constexpr int MAX_TONES = 64;
    ToneMark tone_list[MAX_TONES];
    int tone_count = 0;

    // Build base_buf: original text with 2-layer tone marks removed
    char base_buf[512];
    uint8_t *out = (uint8_t *)base_buf;

    const uint8_t *p = (const uint8_t *)dsc->text;
    uint32_t prev_cp = 0;
    uint32_t cluster_base_cp = 0; // most recent Thai consonant — sets offset per cluster
    int32_t x_acc = 0;            // accumulated advance width (pixels) from area->x1

    while (*p)
    {
        uint32_t cp = utf8_next_cp(&p);
        if (cp == 0)
            break;

        // Track current cluster's base consonant (Thai: 0E01–0E2E)
        if (cp >= 0x0E01 && cp <= 0x0E2E)
            cluster_base_cp = cp;

        if (th_is_tone_mark(cp))
        {
            bool three_layer = th_is_above_vowel(prev_cp);
            if (!three_layer)
            {
                // 2-layer cluster: save tone mark for separate lowered rendering
                if (tone_count < MAX_TONES)
                {
                    tone_list[tone_count].x = area->x1 + x_acc;
                    tone_list[tone_count].cp = cp;
                    tone_list[tone_count].base_cp = cluster_base_cp;
                    tone_count++;
                }
                // Do NOT write to base_buf; skip this character
                prev_cp = cp;
                continue;
            }
            // 3-layer: fall through and write to base_buf normally
        }

        // Accumulate advance width
        lv_font_glyph_dsc_t g;
        uint32_t next_cp_peek = 0;
        {
            const uint8_t *pp = p;
            next_cp_peek = utf8_next_cp(&pp);
        }
        if (lv_font_get_glyph_dsc(dsc->font, &g, cp, next_cp_peek))
            x_acc += (int32_t)g.adv_w; // adv_w in pixels (LVGL v9)

        out += utf8_write_cp(out, cp);
        if (out >= (uint8_t *)base_buf + sizeof(base_buf) - 4)
            break;
        prev_cp = cp;
    }
    *out = '\0';

    // Render base text (3-layer tone marks included, 2-layer tones removed)
    m_textArena.emplace_back(base_buf);
    lv_draw_label_dsc_t base_dsc = *dsc;
    base_dsc.text = m_textArena.back().c_str();
    lv_draw_label(layer, &base_dsc, area);

    // Render each 2-layer tone mark with a per-consonant dynamic offset
    for (int i = 0; i < tone_count; i++)
    {
        // Compute how many pixels to lower the tone mark so it sits just above the consonant.
        //
        // Font coordinate system (positive = up from baseline):
        //   consonant top = cons_g.box_h + cons_g.ofs_y   (pixels above baseline)
        //   tone mark bottom = tone_g.ofs_y               (pixels above baseline)
        //
        // We want: tone_bottom_on_screen ≈ consonant_top_on_screen + 1 px gap
        // Since screen y increases downward, "lower the tone by Δ" means area.y1 += Δ.
        // Δ = tone_bottom - consonant_top - 1
        // Clamp to 0: tall consonants (ห, บ, ด …) where consonant_top ≈ tone_bottom need
        // no adjustment (the font already positions them correctly for 2-layer).

        lv_font_glyph_dsc_t cons_g = {}, tone_g = {};
        lv_font_get_glyph_dsc(dsc->font, &cons_g, tone_list[i].base_cp, 0);
        lv_font_get_glyph_dsc(dsc->font, &tone_g, tone_list[i].cp, 0);

        int32_t cons_top = (int32_t)cons_g.box_h + (int32_t)cons_g.ofs_y;
        int32_t tone_bott = (int32_t)tone_g.ofs_y;
        int32_t drop = tone_bott - cons_top - 1;
        int32_t offset = (drop > 0) ? drop : 0;

        // Build single-character UTF-8 string into arena
        char tbuf[4];
        int bytes = utf8_write_cp((uint8_t *)tbuf, tone_list[i].cp);
        tbuf[bytes] = '\0';
        m_textArena.emplace_back(tbuf, bytes);

        lv_draw_label_dsc_t tone_dsc = *dsc;
        tone_dsc.text = m_textArena.back().c_str();

        lv_area_t tone_area = *area;
        tone_area.x1 = tone_list[i].x;
        tone_area.x2 = tone_list[i].x + fontSize;
        tone_area.y1 = area->y1 + offset;

        lv_draw_label(layer, &tone_dsc, &tone_area);
    }
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

#if LV_USE_VECTOR_GRAPHIC
void LVCanvas::drawVectorPath(const std::vector<VectorPathCommand> &commands,
                              int32_t offsetX, int32_t offsetY,
                              float scaleX, float scaleY,
                              LVColor fillColor, LVColor strokeColor,
                              int32_t strokeWidth, lv_opa_t fillOpa)
{
    if (!m_canvas)
        return;
    lv_layer_t tmp;
    lv_layer_t *layer = acquireLayer(&tmp);

    lv_draw_vector_dsc_t * dsc = lv_draw_vector_dsc_create(layer);
    lv_vector_path_t * path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);

    lv_fpoint_t pos = {0.0f, 0.0f};
    lv_fpoint_t start = {0.0f, 0.0f};

    for (const auto& cmd : commands)
    {
        switch (cmd.type)
        {
        case 'M':
            if (cmd.args.size() >= 2)
            {
                pos.x = cmd.args[0]; pos.y = cmd.args[1];
                start = pos;
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_move_to(path, &pt);
            }
            break;
        case 'L':
            if (cmd.args.size() >= 2)
            {
                pos.x = cmd.args[0]; pos.y = cmd.args[1];
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_line_to(path, &pt);
            }
            break;
        case 'l':
            if (cmd.args.size() >= 2)
            {
                pos.x += cmd.args[0]; pos.y += cmd.args[1];
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_line_to(path, &pt);
            }
            break;
        case 'H':
            if (cmd.args.size() >= 1)
            {
                pos.x = cmd.args[0];
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_line_to(path, &pt);
            }
            break;
        case 'h':
            if (cmd.args.size() >= 1)
            {
                pos.x += cmd.args[0];
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_line_to(path, &pt);
            }
            break;
        case 'V':
            if (cmd.args.size() >= 1)
            {
                pos.y = cmd.args[0];
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_line_to(path, &pt);
            }
            break;
        case 'v':
            if (cmd.args.size() >= 1)
            {
                pos.y += cmd.args[0];
                lv_fpoint_t pt = { pos.x * scaleX + offsetX, pos.y * scaleY + offsetY };
                lv_vector_path_line_to(path, &pt);
            }
            break;
        case 'C':
            if (cmd.args.size() >= 6)
            {
                lv_fpoint_t cp1 = { cmd.args[0] * scaleX + offsetX, cmd.args[1] * scaleY + offsetY };
                lv_fpoint_t cp2 = { cmd.args[2] * scaleX + offsetX, cmd.args[3] * scaleY + offsetY };
                lv_fpoint_t ep = { cmd.args[4] * scaleX + offsetX, cmd.args[5] * scaleY + offsetY };
                lv_vector_path_cubic_to(path, &cp1, &cp2, &ep);
                pos.x = cmd.args[4]; pos.y = cmd.args[5];
            }
            break;
        case 'c':
            if (cmd.args.size() >= 6)
            {
                lv_fpoint_t cp1 = { (pos.x + cmd.args[0]) * scaleX + offsetX, (pos.y + cmd.args[1]) * scaleY + offsetY };
                lv_fpoint_t cp2 = { (pos.x + cmd.args[2]) * scaleX + offsetX, (pos.y + cmd.args[3]) * scaleY + offsetY };
                lv_fpoint_t ep = { (pos.x + cmd.args[4]) * scaleX + offsetX, (pos.y + cmd.args[5]) * scaleY + offsetY };
                lv_vector_path_cubic_to(path, &cp1, &cp2, &ep);
                pos.x += cmd.args[4]; pos.y += cmd.args[5];
            }
            break;
        case 'Z':
        case 'z':
            lv_vector_path_close(path);
            pos = start;
            break;
        default:
            break;
        }
    }

    if (fillOpa > 0)
    {
        lv_draw_vector_dsc_set_fill_color(dsc, fillColor.raw());
        lv_draw_vector_dsc_set_fill_opa(dsc, fillOpa);
    }
    else
    {
        lv_draw_vector_dsc_set_fill_opa(dsc, LV_OPA_TRANSP);
    }

    if (strokeWidth > 0)
    {
        lv_draw_vector_dsc_set_stroke_color(dsc, strokeColor.raw());
        lv_draw_vector_dsc_set_stroke_width(dsc, (float)strokeWidth);
    }

    lv_draw_vector_dsc_add_path(dsc, path);
    lv_draw_vector(dsc);

    lv_vector_path_delete(path);
    lv_draw_vector_dsc_delete(dsc);

    releaseLayer(&tmp);
}
#else
void LVCanvas::drawVectorPath(const std::vector<VectorPathCommand> &commands,
                              int32_t offsetX, int32_t offsetY,
                              float scaleX, float scaleY,
                              LVColor fillColor, LVColor strokeColor,
                              int32_t strokeWidth, lv_opa_t fillOpa)
{
    (void)commands; (void)offsetX; (void)offsetY; (void)scaleX; (void)scaleY;
    (void)fillColor; (void)strokeColor; (void)strokeWidth; (void)fillOpa;
}
#endif
