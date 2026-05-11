#include "LessonPlayer.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "font_thai.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static const char *TAG = "LessonPlayer";

// ── Screen geometry ───────────────────────────────────────────────────────────
static const int32_t SCR_W = 1280;
static const int32_t SCR_H = 800;
static const int32_t HEADER_H = 56;
static const int32_t FOOTER_H = 64;
static const int32_t CONTENT_H = SCR_H - HEADER_H - FOOTER_H;

// ── Singleton ─────────────────────────────────────────────────────────────────
LessonPlayer &LessonPlayer::getInstance()
{
    static LessonPlayer instance;
    return instance;
}

LessonPlayer::LessonPlayer() {}

LessonPlayer::~LessonPlayer()
{
    if (m_lessonRoot)
        cJSON_Delete(m_lessonRoot);
    delete m_screen;
}

// ── File loading ──────────────────────────────────────────────────────────────

bool LessonPlayer::loadLesson(const char *json_path)
{
    ESP_LOGI(TAG, "Loading lesson: %s", json_path);

    // Free previous lesson
    if (m_lessonRoot)
    {
        cJSON_Delete(m_lessonRoot);
        m_lessonRoot = nullptr;
        m_pagesArray = nullptr;
        m_pageCount = 0;
        m_currentPage = 0;
    }

    FILE *f = fopen(json_path, "r");
    if (!f)
    {
        // Try uppercase filename (web upload converts to uppercase)
        char upper_path[256];
        strncpy(upper_path, json_path, sizeof(upper_path) - 1);
        upper_path[sizeof(upper_path) - 1] = '\0';
        // Find the last '/' and uppercase everything after it
        char *fname = strrchr(upper_path, '/');
        if (fname)
        {
            for (char *p = fname + 1; *p; p++)
                *p = toupper((unsigned char)*p);
        }
        f = fopen(upper_path, "r");
        if (f)
            ESP_LOGI(TAG, "Opened uppercase fallback: %s", upper_path);
    }
    if (!f)
    {
        ESP_LOGE(TAG, "Cannot open: %s", json_path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(sz + 1);
    if (!buf)
    {
        fclose(f);
        ESP_LOGE(TAG, "OOM reading lesson");
        return false;
    }

    size_t n = fread(buf, 1, sz, f);
    buf[n] = '\0';
    fclose(f);

    m_lessonRoot = cJSON_Parse(buf);
    free(buf);

    if (!m_lessonRoot)
    {
        ESP_LOGE(TAG, "JSON parse error: %s", cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "?");
        return false;
    }

    m_pagesArray = cJSON_GetObjectItem(m_lessonRoot, "pages");
    if (!m_pagesArray || !cJSON_IsArray(m_pagesArray))
    {
        ESP_LOGE(TAG, "No 'pages' array in lesson JSON");
        cJSON_Delete(m_lessonRoot);
        m_lessonRoot = nullptr;
        return false;
    }

    m_pageCount = cJSON_GetArraySize(m_pagesArray);
    ESP_LOGI(TAG, "Lesson loaded: %s, %d pages", jstr(m_lessonRoot, "title"), m_pageCount);
    return true;
}

// ── show() — entry point ──────────────────────────────────────────────────────

void LessonPlayer::show()
{
    if (!m_lessonRoot || m_pageCount == 0)
    {
        ESP_LOGE(TAG, "No lesson loaded");
        return;
    }

    lvgl_port_lock(0);

    // Re-create screen
    delete m_screen;
    m_screen = new LVScreen();
    lv_obj_t *root = m_screen->getObj();
    lv_obj_set_style_bg_color(root, lv_color_hex(0x0f0f23), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // ── Header bar ────────────────────────────────────────────────────
    m_headerBar = lv_obj_create(root);
    lv_obj_set_size(m_headerBar, SCR_W, HEADER_H);
    lv_obj_set_pos(m_headerBar, 0, 0);
    lv_obj_set_style_bg_color(m_headerBar, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(m_headerBar, 0, 0);
    lv_obj_set_style_radius(m_headerBar, 0, 0);
    lv_obj_set_style_pad_all(m_headerBar, 0, 0);
    lv_obj_clear_flag(m_headerBar, LV_OBJ_FLAG_SCROLLABLE);

    // Close button
    lv_obj_t *btn_close = lv_btn_create(m_headerBar);
    lv_obj_set_size(btn_close, 44, 44);
    lv_obj_align(btn_close, LV_ALIGN_LEFT_MID, 6, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x3a3a5a), 0);
    lv_obj_set_style_radius(btn_close, 22, 0);
    lv_obj_add_event_cb(btn_close, onCloseBtn, LV_EVENT_CLICKED, this);
    lv_obj_t *lbl_x = lv_label_create(btn_close);
    lv_label_set_text(lbl_x, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(lbl_x, lv_color_hex(0xffffff), 0);
    lv_obj_center(lbl_x);

    // Lesson title
    m_titleLabel = lv_label_create(m_headerBar);
    lv_label_set_text(m_titleLabel, jstr(m_lessonRoot, "title_th", jstr(m_lessonRoot, "title")));
    lv_obj_set_style_text_font(m_titleLabel, th_niramit_select(20), 0);
    lv_obj_set_style_text_color(m_titleLabel, lv_color_hex(0x00d4ff), 0);
    lv_obj_align(m_titleLabel, LV_ALIGN_CENTER, 0, 0);

    // Page indicator label (e.g. "2 / 4")
    m_pageIndicator = lv_label_create(m_headerBar);
    lv_obj_set_style_text_font(m_pageIndicator, th_niramit_select(16), 0);
    lv_obj_set_style_text_color(m_pageIndicator, lv_color_hex(0x888899), 0);
    lv_obj_align(m_pageIndicator, LV_ALIGN_RIGHT_MID, -12, 0);

    // ── Content area ──────────────────────────────────────────────────
    m_contentArea = lv_obj_create(root);
    lv_obj_set_size(m_contentArea, SCR_W, CONTENT_H);
    lv_obj_set_pos(m_contentArea, 0, HEADER_H);
    lv_obj_set_style_bg_color(m_contentArea, lv_color_hex(0x0f0f23), 0);
    lv_obj_set_style_bg_opa(m_contentArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(m_contentArea, 0, 0);
    lv_obj_set_style_pad_all(m_contentArea, 0, 0);
    lv_obj_set_style_radius(m_contentArea, 0, 0);
    lv_obj_clear_flag(m_contentArea, LV_OBJ_FLAG_SCROLLABLE);

    // ── Footer / nav bar ──────────────────────────────────────────────
    lv_obj_t *footer = lv_obj_create(root);
    lv_obj_set_size(footer, SCR_W, FOOTER_H);
    lv_obj_set_pos(footer, 0, HEADER_H + CONTENT_H);
    lv_obj_set_style_bg_color(footer, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_radius(footer, 0, 0);
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    // Back button
    m_btnBack = lv_btn_create(footer);
    lv_obj_set_size(m_btnBack, 140, 46);
    lv_obj_align(m_btnBack, LV_ALIGN_LEFT_MID, 16, 0);
    lv_obj_set_style_bg_color(m_btnBack, lv_color_hex(0x2a2a4a), 0);
    lv_obj_set_style_radius(m_btnBack, 8, 0);
    lv_obj_add_event_cb(m_btnBack, onBackBtn, LV_EVENT_CLICKED, this);
    lv_obj_t *lbl_back = lv_label_create(m_btnBack);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT "  กลับ");
    lv_obj_set_style_text_font(lbl_back, th_niramit_select(20), 0);
    lv_obj_set_style_text_color(lbl_back, lv_color_hex(0xccccdd), 0);
    lv_obj_center(lbl_back);

    // Next button
    m_btnNext = lv_btn_create(footer);
    lv_obj_set_size(m_btnNext, 160, 46);
    lv_obj_align(m_btnNext, LV_ALIGN_RIGHT_MID, -16, 0);
    lv_obj_set_style_bg_color(m_btnNext, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_radius(m_btnNext, 8, 0);
    lv_obj_add_event_cb(m_btnNext, onNextBtn, LV_EVENT_CLICKED, this);
    lv_obj_t *lbl_next = lv_label_create(m_btnNext);
    lv_label_set_text(lbl_next, "ต่อไป  " LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(lbl_next, th_niramit_select(20), 0);
    lv_obj_set_style_text_color(lbl_next, lv_color_hex(0xffffff), 0);
    lv_obj_center(lbl_next);

    // Show first page
    m_currentPage = 0;
    showPage(0);

    // Push onto ScreenManager
    ScreenManager &mgr = ScreenManager::getInstance();
    mgr.registerScreen("lesson_player", m_screen);
    mgr.push("lesson_player", LVScreen::Transition::OverLeft, 200);

    lvgl_port_unlock();

    ESP_LOGI(TAG, "LessonPlayer shown");
}

// ── Page navigation ───────────────────────────────────────────────────────────

void LessonPlayer::showPage(int index)
{
    if (index < 0 || index >= m_pageCount)
        return;

    m_currentPage = index;

    // Clear content area
    clearContent();

    // Update indicator label
    char indBuf[16];
    snprintf(indBuf, sizeof(indBuf), "%d / %d", index + 1, m_pageCount);
    lv_label_set_text(m_pageIndicator, indBuf);

    // Update nav buttons
    updateNavButtons();

    // Render page
    cJSON *page = cJSON_GetArrayItem(m_pagesArray, index);
    if (!page)
        return;

    const char *page_type = jstr(page, "page_type");

    if (strcmp(page_type, "cover") == 0)
        buildCoverPage(m_contentArea, page);
    else if (strcmp(page_type, "theory") == 0)
        buildTheoryPage(m_contentArea, page);
    else if (strcmp(page_type, "circuit") == 0)
        buildCircuitPage(m_contentArea, page);
    else if (strcmp(page_type, "verify") == 0)
        buildVerifyPage(m_contentArea, page);
    else
    {
        lv_obj_t *lbl = lv_label_create(m_contentArea);
        lv_label_set_text_fmt(lbl, "Unknown page type: %s", page_type);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xff4444), 0);
        lv_obj_center(lbl);
    }
}

void LessonPlayer::clearContent()
{
    lv_obj_clean(m_contentArea);
    m_outputLed = nullptr;
    m_outputLabel = nullptr;
    m_circuitPage = nullptr;
    m_verifyResultPanel = nullptr;
    memset(m_inputStates, 0, sizeof(m_inputStates));
}

void LessonPlayer::updateNavButtons()
{
    // Back: disabled on first page
    if (m_currentPage == 0)
        lv_obj_add_state(m_btnBack, LV_STATE_DISABLED);
    else
        lv_obj_remove_state(m_btnBack, LV_STATE_DISABLED);

    // Next label: "เสร็จสิ้น" on last page
    lv_obj_t *lbl = lv_obj_get_child(m_btnNext, 0);
    if (lbl)
    {
        if (m_currentPage == m_pageCount - 1)
            lv_label_set_text(lbl, LV_SYMBOL_OK "  เสร็จสิ้น");
        else
            lv_label_set_text(lbl, "ต่อไป  " LV_SYMBOL_RIGHT);
    }
}

void LessonPlayer::close()
{
    lvgl_port_lock(0);
    ScreenManager::getInstance().back(LVScreen::Transition::OverRight, 200);
    lvgl_port_unlock();
}

// ── Cover page ────────────────────────────────────────────────────────────────

void LessonPlayer::buildCoverPage(lv_obj_t *cont, cJSON *page)
{
    const int32_t PAD = 60;
    int32_t y = 60;

    // Big title
    lv_obj_t *t = lv_label_create(cont);
    lv_label_set_text(t, jstr(page, "title"));
    lv_obj_set_style_text_font(t, th_niramit_select(48), 0);
    lv_obj_set_style_text_color(t, lv_color_hex(0x00d4ff), 0);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, y);
    y += 70;

    // Subtitle (Thai)
    lv_obj_t *sub = lv_label_create(cont);
    lv_label_set_text(sub, jstr(page, "subtitle"));
    lv_obj_set_style_text_font(sub, th_niramit_select(28), 0);
    lv_obj_set_style_text_color(sub, lv_color_hex(0x8888bb), 0);
    lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, y);
    y += 56;

    // IC badge
    lv_obj_t *ic_badge = lv_obj_create(cont);
    lv_obj_set_size(ic_badge, 220, 44);
    lv_obj_align(ic_badge, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_set_style_bg_color(ic_badge, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_radius(ic_badge, 22, 0);
    lv_obj_set_style_border_width(ic_badge, 0, 0);
    lv_obj_clear_flag(ic_badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *ic_lbl = lv_label_create(ic_badge);
    lv_label_set_text(ic_lbl, jstr(page, "ic_label"));
    lv_obj_set_style_text_font(ic_lbl, th_niramit_select(20), 0);
    lv_obj_set_style_text_color(ic_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(ic_lbl);
    y += 64;

    // Separator
    y += 20;

    // Objectives panel
    cJSON *objs = cJSON_GetObjectItem(page, "objectives");
    if (objs && cJSON_IsArray(objs))
    {
        lv_obj_t *panel = lv_obj_create(cont);
        lv_obj_set_size(panel, SCR_W - PAD * 2, LV_SIZE_CONTENT);
        lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, y);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0x16213e), 0);
        lv_obj_set_style_radius(panel, 12, 0);
        lv_obj_set_style_border_width(panel, 1, 0);
        lv_obj_set_style_border_color(panel, lv_color_hex(0x3a3a5a), 0);
        lv_obj_set_style_pad_all(panel, 20, 0);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(panel, 10, 0);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

        // Header
        lv_obj_t *hdr = lv_label_create(panel);
        lv_label_set_text(hdr, LV_SYMBOL_BULLET "  จุดประสงค์การเรียนรู้");
        lv_obj_set_style_text_font(hdr, th_niramit_select(24), 0);
        lv_obj_set_style_text_color(hdr, lv_color_hex(0x00d4ff), 0);

        int n = cJSON_GetArraySize(objs);
        for (int i = 0; i < n; i++)
        {
            cJSON *item = cJSON_GetArrayItem(objs, i);
            if (!cJSON_IsString(item))
                continue;

            char buf[128];
            snprintf(buf, sizeof(buf), "  %d.  %s", i + 1, item->valuestring);

            lv_obj_t *row = lv_label_create(panel);
            lv_label_set_text(row, buf);
            lv_obj_set_style_text_font(row, th_niramit_select(20), 0);
            lv_obj_set_style_text_color(row, lv_color_hex(0xccccdd), 0);
            lv_obj_set_width(row, SCR_W - PAD * 2 - 40);
            lv_label_set_long_mode(row, LV_LABEL_LONG_WRAP);
        }
    }
}

// ── Theory page ───────────────────────────────────────────────────────────────

void LessonPlayer::buildTheoryPage(lv_obj_t *cont, cJSON *page)
{
    const int32_t PAD = 48;

    // Scrollable container so long text doesn't clip
    lv_obj_t *scroll = lv_obj_create(cont);
    lv_obj_set_size(scroll, SCR_W, CONTENT_H);
    lv_obj_set_pos(scroll, 0, 0);
    lv_obj_set_style_bg_color(scroll, lv_color_hex(0x0f0f23), 0);
    lv_obj_set_style_border_width(scroll, 0, 0);
    lv_obj_set_style_pad_hor(scroll, PAD, 0);
    lv_obj_set_style_pad_ver(scroll, 24, 0);
    lv_obj_set_style_radius(scroll, 0, 0);
    lv_obj_set_flex_flow(scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(scroll, 20, 0);

    // Page title
    lv_obj_t *title = lv_label_create(scroll);
    lv_label_set_text(title, jstr(page, "title"));
    lv_obj_set_style_text_font(title, th_niramit_select(28), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_width(title, SCR_W - PAD * 2);

    // Body text
    const char *body = jstr(page, "body");
    if (body && *body)
    {
        lv_obj_t *body_lbl = lv_label_create(scroll);
        lv_label_set_text(body_lbl, body);
        lv_obj_set_style_text_font(body_lbl, th_niramit_select(20), 0);
        lv_obj_set_style_text_color(body_lbl, lv_color_hex(0xccccdd), 0);
        lv_obj_set_width(body_lbl, SCR_W - PAD * 2);
        lv_label_set_long_mode(body_lbl, LV_LABEL_LONG_WRAP);
    }

    // Truth table
    cJSON *tt = cJSON_GetObjectItem(page, "truth_table");
    if (tt)
    {
        cJSON *hdrs = cJSON_GetObjectItem(tt, "col_headers");
        cJSON *rows = cJSON_GetObjectItem(tt, "rows");

        if (hdrs && rows && cJSON_IsArray(hdrs) && cJSON_IsArray(rows))
        {
            int cols = cJSON_GetArraySize(hdrs);
            int row_count = cJSON_GetArraySize(rows);

            // Table panel
            lv_obj_t *tpanel = lv_obj_create(scroll);
            lv_obj_set_size(tpanel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_color(tpanel, lv_color_hex(0x16213e), 0);
            lv_obj_set_style_radius(tpanel, 12, 0);
            lv_obj_set_style_border_width(tpanel, 1, 0);
            lv_obj_set_style_border_color(tpanel, lv_color_hex(0x3a3a5a), 0);
            lv_obj_set_style_pad_all(tpanel, 16, 0);
            lv_obj_clear_flag(tpanel, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t *tt_hdr = lv_label_create(tpanel);
            lv_label_set_text(tt_hdr, "Truth Table");
            lv_obj_set_style_text_font(tt_hdr, th_niramit_select(20), 0);
            lv_obj_set_style_text_color(tt_hdr, lv_color_hex(0x00d4ff), 0);
            lv_obj_set_pos(tt_hdr, 0, 0);

            // Use lv_table
            lv_obj_t *table = lv_table_create(tpanel);
            lv_obj_set_pos(table, 0, 36);
            lv_table_set_col_cnt(table, cols);
            lv_table_set_row_cnt(table, row_count + 1); // +1 for header row

            // Column widths: 120px each
            for (int c = 0; c < cols; c++)
                lv_table_set_col_width(table, c, 120);

            // Header row
            for (int c = 0; c < cols; c++)
            {
                cJSON *hdr_item = cJSON_GetArrayItem(hdrs, c);
                lv_table_set_cell_value(table, 0, c,
                                        cJSON_IsString(hdr_item) ? hdr_item->valuestring : "?");
            }

            // Data rows
            for (int r = 0; r < row_count; r++)
            {
                cJSON *row_obj = cJSON_GetArrayItem(rows, r);
                cJSON *ins = cJSON_GetObjectItem(row_obj, "in");
                cJSON *outs = cJSON_GetObjectItem(row_obj, "out");

                int in_count = ins ? cJSON_GetArraySize(ins) : 0;
                int out_count = outs ? cJSON_GetArraySize(outs) : 0;

                // Fill input columns
                for (int c = 0; c < in_count && c < cols; c++)
                {
                    cJSON *v = cJSON_GetArrayItem(ins, c);
                    lv_table_set_cell_value(table, r + 1, c,
                                            (cJSON_IsNumber(v) && v->valueint == 0) ? "0" : "1");
                }
                // Fill output columns (after inputs)
                for (int c = 0; c < out_count; c++)
                {
                    cJSON *v = cJSON_GetArrayItem(outs, c);
                    lv_table_set_cell_value(table, r + 1, in_count + c,
                                            (cJSON_IsNumber(v) && v->valueint == 0) ? "0" : "1");
                }
            }

            // Style the table
            lv_obj_set_style_text_font(table, th_niramit_select(20), LV_PART_ITEMS);
            lv_obj_set_style_text_color(table, lv_color_hex(0xffffff), LV_PART_ITEMS);
            lv_obj_set_style_bg_color(table, lv_color_hex(0x0f0f23), LV_PART_ITEMS);
            lv_obj_set_style_bg_color(table, lv_color_hex(0x0f4c75),
                                      LV_PART_ITEMS | (lv_style_selector_t)LV_STATE_CHECKED); // header highlight
            lv_obj_set_style_border_color(table, lv_color_hex(0x3a3a5a), LV_PART_ITEMS);
            lv_obj_clear_flag(table, LV_OBJ_FLAG_SCROLLABLE);
        }
    }
}

// ── Circuit page ──────────────────────────────────────────────────────────────
// Interactive software simulation: toggle input A → see output Y update live.

// Helper: compute simulated output given gate type and inputs
static bool simGate(const char *gate_type, const bool *ins, int n)
{
    if (strcmp(gate_type, "NOT") == 0 && n >= 1)
        return !ins[0];
    if (strcmp(gate_type, "AND") == 0 && n >= 2)
        return ins[0] && ins[1];
    if (strcmp(gate_type, "OR") == 0 && n >= 2)
        return ins[0] || ins[1];
    if (strcmp(gate_type, "NAND") == 0 && n >= 2)
        return !(ins[0] && ins[1]);
    if (strcmp(gate_type, "NOR") == 0 && n >= 2)
        return !(ins[0] || ins[1]);
    if (strcmp(gate_type, "XOR") == 0 && n >= 2)
        return ins[0] ^ ins[1];
    if (strcmp(gate_type, "XNOR") == 0 && n >= 2)
        return !(ins[0] ^ ins[1]);
    return false;
}

void LessonPlayer::buildCircuitPage(lv_obj_t *cont, cJSON *page)
{
    m_circuitPage = page;
    const char *gate_type = jstr(page, "gate_type", "NOT");

    // Count inputs
    cJSON *inputs_arr = cJSON_GetObjectItem(page, "inputs");
    int n_inputs = (inputs_arr && cJSON_IsArray(inputs_arr))
                       ? cJSON_GetArraySize(inputs_arr)
                       : 1;
    if (n_inputs > MAX_INPUTS)
        n_inputs = MAX_INPUTS;

    // Count outputs (always 1 for basic gates)
    (void)0; // n_outputs unused, output is always 1 LED

    // ── Page title ──────────────────────────────────────────────────
    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, jstr(page, "title"));
    lv_obj_set_style_text_font(title, th_niramit_select(24), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00d4ff), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    // ── Main circuit panel ──────────────────────────────────────────
    // Layout: [Input toggles] ──── [Gate box] ──── [Output LED]
    const int32_t PANEL_W = 900;
    const int32_t PANEL_H = 220 + n_inputs * 60;
    lv_obj_t *panel = lv_obj_create(cont);
    lv_obj_set_size(panel, PANEL_W, PANEL_H);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x3a3a5a), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    // Column positions inside panel
    const int32_t COL_IN = 60;    // left: input toggles
    const int32_t COL_GATE = 360; // center: gate box
    const int32_t COL_OUT = 660;  // right: output LED
    const int32_t ROW_TOP = 40;
    const int32_t ROW_STEP = 80;

    // ── Input toggles ───────────────────────────────────────────────
    for (int i = 0; i < n_inputs; i++)
    {
        const char *in_name = "A";
        if (inputs_arr)
        {
            cJSON *item = cJSON_GetArrayItem(inputs_arr, i);
            if (cJSON_IsString(item))
                in_name = item->valuestring;
        }

        int32_t row_y = ROW_TOP + i * ROW_STEP + (PANEL_H - n_inputs * ROW_STEP) / 2 - 30;

        // Label
        lv_obj_t *lbl = lv_label_create(panel);
        lv_label_set_text(lbl, in_name);
        lv_obj_set_style_text_font(lbl, th_niramit_select(28), 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
        lv_obj_set_pos(lbl, COL_IN, row_y);

        // Toggle switch
        lv_obj_t *sw = lv_switch_create(panel);
        lv_obj_set_size(sw, 80, 40);
        lv_obj_set_pos(sw, COL_IN + 36, row_y + 2);
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x2a2a4a), 0);
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x00aa55), LV_PART_INDICATOR | (lv_style_selector_t)LV_STATE_CHECKED);
        // Store (self, input_index) in user_data: pack pointer + index
        // We use a simple static struct to pass context
        // The user_data stores the LessonPlayer pointer; the input index is encoded in tag
        lv_obj_set_user_data(sw, this);
        // Encode input index in the object tag
        lv_obj_set_style_translate_x(sw, i, 0); // hack: reuse translate_x to store index
        lv_obj_add_event_cb(sw, onInputToggle, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)i);

        // Wire line (horizontal) from switch to gate
        static lv_point_precise_t wire_pts[2];
        wire_pts[0] = {COL_IN + 120, (lv_value_precise_t)(row_y + 22)};
        wire_pts[1] = {COL_GATE, (lv_value_precise_t)(row_y + 22)};
        lv_obj_t *wire = lv_line_create(panel);
        lv_line_set_points(wire, wire_pts, 2);
        lv_obj_set_style_line_color(wire, lv_color_hex(0x00d4ff), 0);
        lv_obj_set_style_line_width(wire, 3, 0);
    }

    // ── Gate box ────────────────────────────────────────────────────
    const int32_t GATE_W = 120;
    const int32_t GATE_H = 80;
    const int32_t gate_y = (PANEL_H - GATE_H) / 2;

    lv_obj_t *gate_box = lv_obj_create(panel);
    lv_obj_set_size(gate_box, GATE_W, GATE_H);
    lv_obj_set_pos(gate_box, COL_GATE, gate_y);
    lv_obj_set_style_bg_color(gate_box, lv_color_hex(0x0f4c75), 0);
    lv_obj_set_style_radius(gate_box, 10, 0);
    lv_obj_set_style_border_color(gate_box, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_border_width(gate_box, 2, 0);
    lv_obj_clear_flag(gate_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *gate_lbl = lv_label_create(gate_box);
    lv_label_set_text(gate_lbl, gate_type);
    lv_obj_set_style_text_font(gate_lbl, th_niramit_select(24), 0);
    lv_obj_set_style_text_color(gate_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(gate_lbl);

    // ── Output wire ─────────────────────────────────────────────────
    static lv_point_precise_t out_wire_pts[2];
    const int32_t gate_cy = gate_y + GATE_H / 2;
    out_wire_pts[0] = {COL_GATE + GATE_W, (lv_value_precise_t)gate_cy};
    out_wire_pts[1] = {COL_OUT - 10, (lv_value_precise_t)gate_cy};
    lv_obj_t *out_wire = lv_line_create(panel);
    lv_line_set_points(out_wire, out_wire_pts, 2);
    lv_obj_set_style_line_color(out_wire, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_style_line_width(out_wire, 3, 0);

    // ── Output LED + label ──────────────────────────────────────────
    cJSON *outs_arr = cJSON_GetObjectItem(page, "outputs");
    const char *out_name = "Y";
    if (outs_arr)
    {
        cJSON *first = cJSON_GetArrayItem(outs_arr, 0);
        if (cJSON_IsString(first))
            out_name = first->valuestring;
    }

    lv_obj_t *out_name_lbl = lv_label_create(panel);
    lv_label_set_text(out_name_lbl, out_name);
    lv_obj_set_style_text_font(out_name_lbl, th_niramit_select(28), 0);
    lv_obj_set_style_text_color(out_name_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_set_pos(out_name_lbl, COL_OUT, gate_cy - 36);

    m_outputLed = lv_led_create(panel);
    lv_obj_set_size(m_outputLed, 48, 48);
    lv_obj_set_pos(m_outputLed, COL_OUT, gate_cy - 14);
    lv_led_set_color(m_outputLed, lv_color_hex(0xff4444));
    lv_led_off(m_outputLed);

    m_outputLabel = lv_label_create(panel);
    lv_label_set_text(m_outputLabel, "0");
    lv_obj_set_style_text_font(m_outputLabel, th_niramit_select(28), 0);
    lv_obj_set_style_text_color(m_outputLabel, lv_color_hex(0xff6666), 0);
    lv_obj_set_pos(m_outputLabel, COL_OUT + 56, gate_cy - 10);

    // Compute initial output
    bool out = simGate(gate_type, m_inputStates, n_inputs);
    if (out)
    {
        lv_led_on(m_outputLed);
        lv_led_set_color(m_outputLed, lv_color_hex(0x00ee55));
        lv_label_set_text(m_outputLabel, "1");
        lv_obj_set_style_text_color(m_outputLabel, lv_color_hex(0x00ee55), 0);
    }

    // Hint
    lv_obj_t *hint = lv_label_create(cont);
    lv_label_set_text(hint, jstr(page, "hint"));
    lv_obj_set_style_text_font(hint, th_niramit_select(20), 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888899), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);
}

// ── Verify page ───────────────────────────────────────────────────────────────

void LessonPlayer::buildVerifyPage(lv_obj_t *cont, cJSON *page)
{
    const int32_t PAD = 48;
    int32_t y = 24;

    // Title
    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, jstr(page, "title"));
    lv_obj_set_style_text_font(title, th_niramit_select(28), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xf39c12), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, y);
    y += 56;

    // Instructions panel
    cJSON *instrs = cJSON_GetObjectItem(page, "instructions");
    if (instrs && cJSON_IsArray(instrs))
    {
        lv_obj_t *panel = lv_obj_create(cont);
        lv_obj_set_size(panel, SCR_W - PAD * 2, LV_SIZE_CONTENT);
        lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, y);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0x16213e), 0);
        lv_obj_set_style_radius(panel, 12, 0);
        lv_obj_set_style_border_color(panel, lv_color_hex(0x3a3a5a), 0);
        lv_obj_set_style_border_width(panel, 1, 0);
        lv_obj_set_style_pad_all(panel, 20, 0);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(panel, 10, 0);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *hdr = lv_label_create(panel);
        lv_label_set_text(hdr, "วิธีต่อวงจร:");
        lv_obj_set_style_text_font(hdr, th_niramit_select(20), 0);
        lv_obj_set_style_text_color(hdr, lv_color_hex(0x00d4ff), 0);

        int n = cJSON_GetArraySize(instrs);
        for (int i = 0; i < n; i++)
        {
            cJSON *item = cJSON_GetArrayItem(instrs, i);
            if (!cJSON_IsString(item))
                continue;

            char buf[128];
            snprintf(buf, sizeof(buf), "%d.  %s", i + 1, item->valuestring);

            lv_obj_t *row = lv_label_create(panel);
            lv_label_set_text(row, buf);
            lv_obj_set_style_text_font(row, th_niramit_select(20), 0);
            lv_obj_set_style_text_color(row, lv_color_hex(0xccccdd), 0);
            lv_obj_set_width(row, SCR_W - PAD * 2 - 40);
            lv_label_set_long_mode(row, LV_LABEL_LONG_WRAP);
        }
    }

    // Test button
    lv_obj_t *test_btn = lv_btn_create(cont);
    lv_obj_set_size(test_btn, 260, 64);
    lv_obj_align(test_btn, LV_ALIGN_BOTTOM_MID, 0, -100);
    lv_obj_set_style_bg_color(test_btn, lv_color_hex(0x1a5e2a), 0);
    lv_obj_set_style_radius(test_btn, 12, 0);
    lv_obj_add_event_cb(test_btn, onVerifyBtn, LV_EVENT_CLICKED, this);

    lv_obj_t *test_lbl = lv_label_create(test_btn);
    lv_label_set_text(test_lbl, LV_SYMBOL_PLAY "  เริ่มทดสอบ");
    lv_obj_set_style_text_font(test_lbl, th_niramit_select(24), 0);
    lv_obj_set_style_text_color(test_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(test_lbl);

    // Result panel (initially hidden)
    m_verifyResultPanel = lv_obj_create(cont);
    lv_obj_set_size(m_verifyResultPanel, SCR_W - PAD * 2, 80);
    lv_obj_align(m_verifyResultPanel, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_style_bg_color(m_verifyResultPanel, lv_color_hex(0x1a5e2a), 0);
    lv_obj_set_style_radius(m_verifyResultPanel, 12, 0);
    lv_obj_set_style_border_width(m_verifyResultPanel, 0, 0);
    lv_obj_clear_flag(m_verifyResultPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(m_verifyResultPanel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *res_lbl = lv_label_create(m_verifyResultPanel);
    lv_label_set_text(res_lbl, LV_SYMBOL_OK "  UartBridge: เชื่อมต่อ H7 ในขั้นตอนถัดไป");
    lv_obj_set_style_text_font(res_lbl, th_niramit_select(20), 0);
    lv_obj_set_style_text_color(res_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(res_lbl);
}

// ── Event callbacks ───────────────────────────────────────────────────────────

void LessonPlayer::onNextBtn(lv_event_t *e)
{
    LessonPlayer *self = static_cast<LessonPlayer *>(lv_event_get_user_data(e));
    if (self->m_currentPage < self->m_pageCount - 1)
        self->showPage(self->m_currentPage + 1);
    else
        self->close();
}

void LessonPlayer::onBackBtn(lv_event_t *e)
{
    LessonPlayer *self = static_cast<LessonPlayer *>(lv_event_get_user_data(e));
    if (self->m_currentPage > 0)
        self->showPage(self->m_currentPage - 1);
}

void LessonPlayer::onCloseBtn(lv_event_t *e)
{
    LessonPlayer *self = static_cast<LessonPlayer *>(lv_event_get_user_data(e));
    self->close();
}

void LessonPlayer::onInputToggle(lv_event_t *e)
{
    LessonPlayer *self = static_cast<LessonPlayer *>(lv_obj_get_user_data(lv_event_get_target_obj(e)));
    int idx = (int)(intptr_t)lv_event_get_user_data(e);

    lv_obj_t *sw = lv_event_get_target_obj(e);
    bool is_on = lv_obj_has_state(sw, LV_STATE_CHECKED);

    if (idx >= 0 && idx < MAX_INPUTS)
        self->m_inputStates[idx] = is_on;

    // Recompute output
    if (!self->m_circuitPage || !self->m_outputLed || !self->m_outputLabel)
        return;

    cJSON *inputs_arr = cJSON_GetObjectItem(self->m_circuitPage, "inputs");
    int n_inputs = (inputs_arr && cJSON_IsArray(inputs_arr))
                       ? cJSON_GetArraySize(inputs_arr)
                       : 1;
    if (n_inputs > MAX_INPUTS)
        n_inputs = MAX_INPUTS;

    const char *gate_type = jstr(self->m_circuitPage, "gate_type", "NOT");
    bool out = simGate(gate_type, self->m_inputStates, n_inputs);

    if (out)
    {
        lv_led_on(self->m_outputLed);
        lv_led_set_color(self->m_outputLed, lv_color_hex(0x00ee55));
        lv_label_set_text(self->m_outputLabel, "1");
        lv_obj_set_style_text_color(self->m_outputLabel, lv_color_hex(0x00ee55), 0);
    }
    else
    {
        lv_led_off(self->m_outputLed);
        lv_led_set_color(self->m_outputLed, lv_color_hex(0xff4444));
        lv_label_set_text(self->m_outputLabel, "0");
        lv_obj_set_style_text_color(self->m_outputLabel, lv_color_hex(0xff6666), 0);
    }
}

void LessonPlayer::onVerifyBtn(lv_event_t *e)
{
    LessonPlayer *self = static_cast<LessonPlayer *>(lv_event_get_user_data(e));
    // Stub: UartBridge integration in next sprint
    if (self->m_verifyResultPanel)
        lv_obj_remove_flag(self->m_verifyResultPanel, LV_OBJ_FLAG_HIDDEN);

    ESP_LOGI(TAG, "Verify button pressed — UartBridge stub");
}

// ── Helpers ───────────────────────────────────────────────────────────────────

const char *LessonPlayer::jstr(cJSON *obj, const char *key, const char *fallback)
{
    if (!obj)
        return fallback;
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && cJSON_IsString(item) && item->valuestring)
        return item->valuestring;
    return fallback;
}
