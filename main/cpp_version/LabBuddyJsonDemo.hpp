#pragma once

#include "LVWidget.hpp"
#include "LVLabel.hpp"
#include "LVCanvas.hpp"
#include "JsonRenderer.hpp"
#include <memory>

/**
 * @brief LabBuddy JSON Demo - Load and render circuit from JSON file
 *
 * This demo demonstrates loading and rendering circuit diagrams
 * from JSON files on SD card.
 */
class LabBuddyJsonDemo : public LVWidget
{
public:
    LabBuddyJsonDemo(LVWidget *parent = nullptr);
    ~LabBuddyJsonDemo();

    /**
     * @brief Initialize demo
     */
    void init();

    /**
     * @brief Load and render JSON file
     * @param filePath Path to JSON file (e.g., "/sdcard/worksheets/circuit.json")
     * @return true on success
     */
    bool loadCircuit(const char *filePath);

private:
    void createCanvas();
    void listSDCardFiles(const char* path);

    static constexpr int CANVAS_WIDTH = 1024;
    static constexpr int CANVAS_HEIGHT = 500;

    LVCanvas *m_canvas;
    LVLabel *m_titleLabel;
    LVLabel *m_statusLabel;
    std::unique_ptr<JsonRenderer::JsonRenderer> m_renderer;
    uint8_t *m_canvasBuffer;
};
