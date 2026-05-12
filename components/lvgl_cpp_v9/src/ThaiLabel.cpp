/**
 * ThaiLabel.cpp — Thai-aware LVGL label widget
 *
 * Implements the same 2-layer tone-mark shaper as LVCanvas::drawTextThaiShaped().
 * The widget is a plain lv_obj with a LV_EVENT_DRAW_MAIN callback; LVGL itself
 * never draws text — our callback handles everything.
 *
 * Sizing: LV_EVENT_GET_SELF_SIZE is handled so lv_obj_set_size(…, LV_SIZE_CONTENT)
 * works correctly.  Extra vertical space is added for above-baseline tone marks.
 */
#include "ThaiLabel.h"
#include "font_thai.h"

#include "../../managed_components/lvgl__lvgl/src/draw/lv_draw_label.h"
#include "../../managed_components/lvgl__lvgl/src/misc/lv_text.h"
#include "../../managed_components/lvgl__lvgl/src/misc/lv_text_private.h"

#include <cstring>
#include <string>

// ─── Per-widget state ─────────────────────────────────────────────────────────

struct ThaiLabelData
{
    std::string text;
    const lv_font_t *font = nullptr;
    lv_color_t color = {};
    lv_text_align_t align = LV_TEXT_ALIGN_CENTER;
    // shaped_arena removed: text_local=1 makes each lv_draw_label task own
    // a heap copy of its text, so string lifetime is tied to the draw task.
};

// ─── Thai character helpers (mirrors LVCanvas.cpp) ───────────────────────────

static inline bool th_is_above_vowel(uint32_t cp)
{
    return (cp == 0x0E31) ||
           (cp >= 0x0E34 && cp <= 0x0E37) ||
           (cp == 0x0E47) ||
           (cp == 0x0E4D);
}

static inline bool th_is_tone_mark(uint32_t cp)
{
    return (cp >= 0x0E48 && cp <= 0x0E4B);
}

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
    (*p) += 3;
    return 0xFFFD;
}

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

// ─── Core shaper (free function) ─────────────────────────────────────────────

/**
 * Draw @p text using Thai tone-mark shaping into @p layer.
 *
 * Strategy:
 *   - Build base_buf = original text with 2-layer tone marks stripped.
 *     TH Niramit tone marks have adv_w≈0 so line-break points are identical.
 *   - Walk original text LINE-BY-LINE (same wrap width as lv_draw_label will use).
 *     Per-line: compute alignment offset and x_acc cursor tracking.
 *     Record each tone mark with the CURSOR POSITION AFTER ITS CONSONANT and
 *     the absolute y1 of the line.  The font's own ofs_x (negative) is left to
 *     overlay the tone glyph onto the consonant — we must NOT subtract
 *     prev_adv here because ofs_x already encodes that offset.
 *   - Render base_buf with lv_draw_label (handles alignment & wrapping).
 *   - For each recorded tone mark, render it via a separate lv_draw_label call
 *     with LEFT alignment so pos.x == tone_area.x1 == cursor-after-consonant.
 *     Each task uses text_local=1 (LVGL heap-copies) for pipeline safety.
 */
static void thai_draw_shaped(lv_layer_t *layer,
                             const lv_draw_label_dsc_t *dsc,
                             const lv_area_t *area)
{
    struct ToneMark
    {
        int32_t abs_x;    // cursor after consonant, absolute screen coords
        int32_t abs_y1;   // line top, absolute screen coords
        uint32_t cp;      // tone mark codepoint
        uint32_t base_cp; // preceding consonant codepoint (for vert-offset calc)
    };

    static constexpr int MAX_TONES = 64;
    ToneMark tone_list[MAX_TONES];
    int tone_count = 0;

    int32_t area_w = area->x2 - area->x1 + 1;
    int32_t line_h = dsc->font->line_height + dsc->line_space;

    // ── Pass 1: build base_buf by stripping 2-layer tone marks ────────────────
    char base_buf[512];
    uint8_t *base_out = (uint8_t *)base_buf;
    {
        const uint8_t *p = (const uint8_t *)dsc->text;
        uint32_t prev = 0;
        while (*p)
        {
            uint32_t cp = utf8_next_cp(&p);
            if (!cp)
                break;
            if (th_is_tone_mark(cp) && !th_is_above_vowel(prev))
            {
                prev = cp;
                continue;
            }
            base_out += utf8_write_cp(base_out, cp);
            if (base_out >= (uint8_t *)base_buf + sizeof(base_buf) - 4)
                break;
            prev = cp;
        }
        *base_out = '\0';
    }
    uint32_t base_len = (uint32_t)(base_out - (uint8_t *)base_buf);

    // ── Pass 2: walk original text line-by-line to record tone mark positions ─
    //
    // lv_text_get_next_line wraps on adv_w; TH Niramit tone marks have adv_w=0
    // so wrapping of the original text equals wrapping of base_buf. ✓
    const char *txt = dsc->text;
    uint32_t full_len = dsc->text_length > 0
                            ? dsc->text_length
                            : (uint32_t)lv_strlen(txt);
    uint32_t line_start = 0;
    int32_t cursor_y = 0; // relative to area->y1

    while (line_start < full_len && txt[line_start] != '\0')
    {
        // Find where this line ends (using same wrap width as lv_draw_label)
        lv_text_attributes_t lat = {};
        lat.letter_space = dsc->letter_space;
        lat.line_space = dsc->line_space;
        lat.max_width = area_w;
        lat.text_flags = dsc->flag;

        uint32_t remaining = full_len - line_start;
        int32_t used_w = 0;
        uint32_t line_len = lv_text_get_next_line(
            &txt[line_start], remaining, dsc->font, &used_w, &lat);
        if (line_len == 0)
            break;

        // Compute per-line alignment offset
        // (tone marks have adv_w=0 → lv_text_get_width on original line = base line width)
        int32_t align_ofs = 0;
        if (dsc->align != LV_TEXT_ALIGN_LEFT)
        {
            lv_text_attributes_t wa = {};
            wa.letter_space = dsc->letter_space;
            wa.max_width = LV_COORD_MAX;
            // Exclude trailing \n/\r from width measurement
            uint32_t mlen = line_len;
            while (mlen > 0 && (txt[line_start + mlen - 1] == '\n' ||
                                txt[line_start + mlen - 1] == '\r'))
                mlen--;
            int32_t lw = lv_text_get_width(&txt[line_start], mlen, dsc->font, &wa);
            align_ofs = (dsc->align == LV_TEXT_ALIGN_CENTER) ? (area_w - lw) / 2
                                                             : area_w - lw;
            if (align_ofs < 0)
                align_ofs = 0;
        }

        // Walk this line's codepoints, track cursor x, record tone marks
        int tone_line_start = tone_count;
        const uint8_t *lp = (const uint8_t *)&txt[line_start];
        const uint8_t *lend = lp + line_len;
        uint32_t prev_cp = 0;
        uint32_t cluster_base = 0;
        int32_t x_acc = 0;

        while (lp < lend)
        {
            if (*lp == '\n' || *lp == '\r' || *lp == '\0')
                break;
            uint32_t cp = utf8_next_cp(&lp);
            if (!cp || cp == '\n' || cp == '\r')
                break;

            if (cp >= 0x0E01 && cp <= 0x0E2E)
                cluster_base = cp;

            if (th_is_tone_mark(cp) && !th_is_above_vowel(prev_cp))
            {
                if (tone_count < MAX_TONES)
                {
                    // Record cursor AFTER consonant (x_acc already includes consonant adv_w).
                    // We do NOT subtract prev_adv: the font's ofs_x (negative) handles
                    // the horizontal overlay onto the consonant.
                    tone_list[tone_count++] = {
                        x_acc,    // relative to line text start; patched below
                        cursor_y, // relative to area->y1; patched below
                        cp, cluster_base};
                }
                prev_cp = cp;
                continue; // tone marks have adv_w≈0; do NOT advance x_acc
            }

            lv_font_glyph_dsc_t g;
            const uint8_t *pp = lp;
            uint32_t nxt = utf8_next_cp(&pp);
            int32_t adv = 0;
            if (lv_font_get_glyph_dsc(dsc->font, &g, cp, nxt))
                adv = g.adv_w;
            x_acc += adv;
            prev_cp = cp;
        }

        // Patch tone marks to absolute screen coordinates
        for (int i = tone_line_start; i < tone_count; i++)
        {
            tone_list[i].abs_x = area->x1 + align_ofs + tone_list[i].abs_x;
            tone_list[i].abs_y1 = area->y1 + tone_list[i].abs_y1;
        }

        line_start += line_len;
        cursor_y += line_h;
    }

    // ── Render base text ──────────────────────────────────────────────────────
    lv_draw_label_dsc_t base_dsc = *dsc;
    base_dsc.text = base_buf;
    base_dsc.text_length = base_len;
    base_dsc.text_local = 1; // LVGL heap-copies; safe across pipeline frames
    lv_draw_label(layer, &base_dsc, area);

    // ── Render each 2-layer tone mark ─────────────────────────────────────────
    for (int i = 0; i < tone_count; i++)
    {
        lv_font_glyph_dsc_t cons_g = {}, tone_g = {};
        lv_font_get_glyph_dsc(dsc->font, &cons_g, tone_list[i].base_cp, 0);
        lv_font_get_glyph_dsc(dsc->font, &tone_g, tone_list[i].cp, 0);

        // Vertical adjustment: push tone mark up if it would overlap consonant cap
        int32_t cons_top = (int32_t)cons_g.box_h + (int32_t)cons_g.ofs_y;
        int32_t tone_bott = (int32_t)tone_g.ofs_y;
        int32_t drop = tone_bott - cons_top - 1;
        int32_t y_offset = (drop > 0) ? drop : 0;

        char tbuf[4];
        int bytes = utf8_write_cp((uint8_t *)tbuf, tone_list[i].cp);
        tbuf[bytes] = '\0';

        lv_draw_label_dsc_t tone_dsc = *dsc;
        tone_dsc.text = tbuf;
        tone_dsc.text_length = (uint32_t)bytes;
        tone_dsc.text_local = 1;
        // LEFT: pos.x == tone_area.x1 == cursor-after-consonant.
        // Font's ofs_x (negative) overlays the glyph onto the consonant.
        tone_dsc.align = LV_TEXT_ALIGN_LEFT;

        lv_area_t tone_area = *area;
        tone_area.x1 = tone_list[i].abs_x;
        tone_area.x2 = tone_list[i].abs_x + dsc->font->line_height; // generous width
        tone_area.y1 = tone_list[i].abs_y1 + y_offset;

        lv_draw_label(layer, &tone_dsc, &tone_area);
    }
}

// ─── LVGL event callbacks ─────────────────────────────────────────────────────

static ThaiLabelData *get_data(lv_obj_t *obj)
{
    return (ThaiLabelData *)lv_obj_get_user_data(obj);
}

static void thai_label_draw_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_DRAW_MAIN)
        return;

    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    ThaiLabelData *d = get_data(obj);
    if (!d || d->text.empty() || !d->font)
        return;

    lv_layer_t *layer = lv_event_get_layer(e);

    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    // Guard: zero-width or zero-height area causes infinite loop in lv_draw_label
    if (coords.x2 <= coords.x1 || coords.y2 <= coords.y1)
        return;

    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = d->color;
    dsc.font = d->font;
    dsc.text = d->text.c_str();
    dsc.align = d->align;

    // text_local=1 inside thai_draw_shaped makes each lv_draw_label task own
    // a heap copy of its text — no arena needed, safe with LVGL pipeline rendering.
    thai_draw_shaped(layer, &dsc, &coords);
}

static void thai_label_size_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_GET_SELF_SIZE)
        return;

    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    ThaiLabelData *d = get_data(obj);
    if (!d || !d->font || d->text.empty())
        return;

    lv_point_t *sz = (lv_point_t *)lv_event_get_param(e);
    lv_point_t text_sz;
    lv_text_get_size(&text_sz, d->text.c_str(), d->font,
                     0, 0, 0x7fff, LV_TEXT_FLAG_NONE);
    // Extra top margin so above-baseline tone marks render within the object area
    sz->x = text_sz.x;
    sz->y = text_sz.y + d->font->line_height / 4 + 4;
}

static void thai_label_delete_cb(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    ThaiLabelData *d = get_data(obj);
    delete d;
    lv_obj_set_user_data(obj, nullptr);
}

// ─── Public API ───────────────────────────────────────────────────────────────

extern "C"
{

    lv_obj_t *thai_label_create(lv_obj_t *parent)
    {
        lv_obj_t *obj = lv_obj_create(parent);

        // Transparent, no border, no padding, not interactive
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(obj, 0, 0);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        ThaiLabelData *d = new ThaiLabelData();
        d->font = th_niramit_select(24);
        d->color = lv_color_white();
        d->align = LV_TEXT_ALIGN_CENTER;

        lv_obj_set_user_data(obj, d);
        lv_obj_add_event_cb(obj, thai_label_draw_cb, LV_EVENT_DRAW_MAIN, nullptr);
        lv_obj_add_event_cb(obj, thai_label_size_cb, LV_EVENT_GET_SELF_SIZE, nullptr);
        lv_obj_add_event_cb(obj, thai_label_delete_cb, LV_EVENT_DELETE, nullptr);
        return obj;
    }

    void thai_label_set_text(lv_obj_t *obj, const char *text)
    {
        ThaiLabelData *d = get_data(obj);
        if (!d)
            return;
        d->text = text ? text : "";
        lv_obj_invalidate(obj);
    }

    void thai_label_set_font(lv_obj_t *obj, const lv_font_t *font)
    {
        ThaiLabelData *d = get_data(obj);
        if (!d)
            return;
        d->font = font;
        lv_obj_invalidate(obj);
    }

    void thai_label_set_color(lv_obj_t *obj, lv_color_t color)
    {
        ThaiLabelData *d = get_data(obj);
        if (!d)
            return;
        d->color = color;
        lv_obj_invalidate(obj);
    }

    void thai_label_set_align(lv_obj_t *obj, lv_text_align_t align)
    {
        ThaiLabelData *d = get_data(obj);
        if (!d)
            return;
        d->align = align;
        lv_obj_invalidate(obj);
    }

} // extern "C"
