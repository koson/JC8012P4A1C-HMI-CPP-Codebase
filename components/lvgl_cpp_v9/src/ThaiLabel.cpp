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
 * 2-layer tone marks are removed from the base run and redrawn at a
 * font-metric-derived vertical offset so they sit just above the consonant.
 *
 * Each lv_draw_label call uses text_local=1 so LVGL heap-copies the text
 * string into the draw task — no external arena needed.
 */
static void thai_draw_shaped(lv_layer_t *layer,
                             const lv_draw_label_dsc_t *dsc,
                             const lv_area_t *area)
{
    struct ToneMark
    {
        int32_t x;
        uint32_t cp;
        uint32_t base_cp;
    };
    static constexpr int MAX_TONES = 64;
    ToneMark tone_list[MAX_TONES];
    int tone_count = 0;

    char base_buf[512];
    uint8_t *out = (uint8_t *)base_buf;

    const uint8_t *p = (const uint8_t *)dsc->text;
    uint32_t prev_cp = 0;
    uint32_t cluster_base = 0;
    int32_t x_acc = 0; // pixel advance from area->x1

    while (*p)
    {
        uint32_t cp = utf8_next_cp(&p);
        if (cp == 0)
            break;

        if (cp >= 0x0E01 && cp <= 0x0E2E)
            cluster_base = cp;

        if (th_is_tone_mark(cp))
        {
            if (!th_is_above_vowel(prev_cp))
            {
                if (tone_count < MAX_TONES)
                {
                    tone_list[tone_count++] = {area->x1 + x_acc, cp, cluster_base};
                }
                prev_cp = cp;
                continue; // skip from base text
            }
            // 3-layer cluster: include normally
        }

        // Accumulate advance
        lv_font_glyph_dsc_t g;
        const uint8_t *pp = p;
        uint32_t next_peek = utf8_next_cp(&pp);
        if (lv_font_get_glyph_dsc(dsc->font, &g, cp, next_peek))
            x_acc += (int32_t)g.adv_w;

        out += utf8_write_cp(out, cp);
        if (out >= (uint8_t *)base_buf + sizeof(base_buf) - 4)
            break;
        prev_cp = cp;
    }
    *out = '\0';

    // Draw base text (tone marks removed).
    // text_local=1: lv_draw_label heap-copies the text into the draw task.
    // text_length: byte count so remaining_len is correct in lv_draw_label_iterate_characters.
    lv_draw_label_dsc_t base_dsc = *dsc;
    base_dsc.text = base_buf;
    base_dsc.text_length = (uint32_t)(out - (uint8_t *)base_buf);
    base_dsc.text_local = 1;
    lv_draw_label(layer, &base_dsc, area);

    // Draw each 2-layer tone mark at computed offset
    for (int i = 0; i < tone_count; i++)
    {
        lv_font_glyph_dsc_t cons_g = {}, tone_g = {};
        lv_font_get_glyph_dsc(dsc->font, &cons_g, tone_list[i].base_cp, 0);
        lv_font_get_glyph_dsc(dsc->font, &tone_g, tone_list[i].cp, 0);

        int32_t cons_top = (int32_t)cons_g.box_h + (int32_t)cons_g.ofs_y;
        int32_t tone_bott = (int32_t)tone_g.ofs_y;
        int32_t drop = tone_bott - cons_top - 1;
        int32_t offset = (drop > 0) ? drop : 0;

        char tbuf[4];
        int bytes = utf8_write_cp((uint8_t *)tbuf, tone_list[i].cp);
        tbuf[bytes] = '\0';

        // text_local=1: lv_draw_label heap-copies tbuf (stack) into the task.
        lv_draw_label_dsc_t tone_dsc = *dsc;
        tone_dsc.text = tbuf;
        tone_dsc.text_length = (uint32_t)bytes;
        tone_dsc.text_local = 1;

        int32_t font_sz = dsc->font->line_height;
        lv_area_t tone_area = *area;
        tone_area.x1 = tone_list[i].x;
        tone_area.x2 = tone_list[i].x + font_sz;
        tone_area.y1 = area->y1 + offset;

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
