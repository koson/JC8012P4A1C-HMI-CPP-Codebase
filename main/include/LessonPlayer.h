#pragma once

#include "LVScreen.hpp"
#include "ScreenManager.hpp"
#include "cJSON.h"
#include <stdint.h>
#include <memory>

class LVCanvas;
namespace JsonRenderer
{
    class JsonRenderer;
}

/**
 * @brief Multi-page lesson player for LabBuddy
 *
 * Loads a lesson JSON from SD card and renders each page as an interactive
 * LVGL screen.  Pages types supported:
 *   - cover   : title, subtitle, objectives
 *   - theory  : text body + truth table
 *   - circuit : interactive gate simulation (toggle input → see output)
 *   - verify  : hardware test via UartBridge (stub until H7 track is ready)
 *
 * Usage:
 * @code
 *   if (LessonPlayer::getInstance().loadLesson("/sdcard/lessons/L001_not_gate.json")) {
 *       LessonPlayer::getInstance().show();
 *   }
 * @endcode
 */
class LessonPlayer
{
public:
    static LessonPlayer &getInstance();

    /**
     * @brief Load a lesson JSON file from SD card.
     * @param json_path  Full path, e.g. "/sdcard/lessons/L001_not_gate.json"
     * @return true on success
     */
    bool loadLesson(const char *json_path);

    /**
     * @brief Show the lesson (creates a new LVScreen and pushes it via ScreenManager).
     *        Must be called after loadLesson().
     */
    void show();

    /**
     * @brief Close / pop the lesson screen.
     */
    void close();

private:
    LessonPlayer();
    ~LessonPlayer();
    LessonPlayer(const LessonPlayer &) = delete;
    LessonPlayer &operator=(const LessonPlayer &) = delete;

    // ── Page rendering ────────────────────────────────────────────────
    void showPage(int index);
    void clearContent();
    void updateNavButtons();

    void buildCoverPage(lv_obj_t *cont, cJSON *page);
    void buildTheoryPage(lv_obj_t *cont, cJSON *page);
    void buildCircuitPage(lv_obj_t *cont, cJSON *page);
    void buildVerifyPage(lv_obj_t *cont, cJSON *page);
    bool tryRenderCircuitFromJson(lv_obj_t *cont, cJSON *page);
    bool resolveCircuitPath(const char *raw_path, char *resolved_path, size_t resolved_size) const;
    void releaseCircuitRenderer();

    // ── Static LVGL callbacks ─────────────────────────────────────────
    static void onNextBtn(lv_event_t *e);
    static void onBackBtn(lv_event_t *e);
    static void onCloseBtn(lv_event_t *e);
    static void onInputToggle(lv_event_t *e);
    static void onVerifyBtn(lv_event_t *e);

    // ── Helper: safe cJSON string getter ─────────────────────────────
    static const char *jstr(cJSON *obj, const char *key, const char *fallback = "");

    // ── LVGL object handles ───────────────────────────────────────────
    LVScreen *m_screen = nullptr;
    lv_obj_t *m_headerBar = nullptr;
    lv_obj_t *m_titleLabel = nullptr;
    lv_obj_t *m_pageIndicator = nullptr;
    lv_obj_t *m_contentArea = nullptr;
    lv_obj_t *m_btnBack = nullptr;
    lv_obj_t *m_btnNext = nullptr;

    // Circuit page: input toggle states + output indicator
    static constexpr int MAX_INPUTS = 4;
    bool m_inputStates[MAX_INPUTS] = {};
    lv_obj_t *m_outputLed = nullptr;   ///< lv_led for circuit page
    lv_obj_t *m_outputLabel = nullptr; ///< "1" / "0" label beside LED
    cJSON *m_circuitPage = nullptr;    ///< cached page JSON for circuit rebuild

    // Verify page
    lv_obj_t *m_verifyResultPanel = nullptr;

    // ── Lesson data ───────────────────────────────────────────────────
    cJSON *m_lessonRoot = nullptr;
    cJSON *m_pagesArray = nullptr;
    int m_currentPage = 0;
    int m_pageCount = 0;
    char m_loadedLessonPath[256] = {};

    // Optional circuit JSON renderer on circuit page
    LVCanvas *m_circuitCanvas = nullptr;
    void *m_circuitCanvasBuffer = nullptr;
    void *m_circuitBackBuffer = nullptr;
    std::unique_ptr<JsonRenderer::JsonRenderer> m_circuitRenderer;
};
