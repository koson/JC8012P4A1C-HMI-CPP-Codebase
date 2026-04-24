#pragma once

#include "lvgl.h"
#include "JsonRenderer.hpp"
#include "LVCanvas.hpp"
#include <vector>
#include <string>
#include <memory>

/**
 * @brief FileViewerUI - Simple LVGL UI for FileManager
 *
 * Displays:
 * - WiFi IP address
 * - File list from SD card
 * - Refresh button
 * - Render button
 * - Canvas for displaying JSON/SVG
 */
class FileViewerUI
{
public:
    FileViewerUI();
    ~FileViewerUI();

    /**
     * @brief Create UI on parent object
     * @param parent LVGL parent object (usually lv_screen_active())
     */
    void create(lv_obj_t *parent);

    /**
     * @brief Update IP address label
     * @param ip IP address string (e.g., "192.168.1.41")
     */
    void updateIP(const char *ip);

    /**
     * @brief Refresh file list from SD card
     * Scans /sdcard/WORKSHOP/ and populates list
     */
    void refreshFileList();

    /**
     * @brief Render selected file
     * Gets selected file from list and renders to canvas
     */
    void renderSelected();

    /**
     * @brief Clear canvas
     */
    void clearCanvas();

private:
    // Event handlers (static for C callback compatibility)
    static void btnRefreshClicked(lv_event_t *e);
    static void btnRenderClicked(lv_event_t *e);
    static void btnClearClicked(lv_event_t *e);
    static void fileItemClicked(lv_event_t *e);

    // UI components
    lv_obj_t *m_parent;
    lv_obj_t *m_labelIP;
    lv_obj_t *m_labelStatus;
    lv_obj_t *m_fileList;
    lv_obj_t *m_btnRefresh;
    lv_obj_t *m_btnRender;
    lv_obj_t *m_btnClear;
    lv_obj_t *m_canvasContainer;

    // Canvas and renderer
    LVCanvas *m_canvas;
    void *m_canvasBuffer;
    std::unique_ptr<JsonRenderer::JsonRenderer> m_renderer;

    // File list data
    struct FileItem
    {
        std::string name;
        size_t size;
        bool selected;
    };
    std::vector<FileItem> m_files;
    int m_selectedIndex;

    // Helper methods
    void scanFiles();
    void updateFileListUI();
    std::string getSelectedFilePath();
};
