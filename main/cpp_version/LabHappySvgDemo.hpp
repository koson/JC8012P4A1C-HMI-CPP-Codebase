#pragma once

#include "LVWidget.hpp"
#include "LVCanvas.hpp"
#include "LVLabel.hpp"
#include "SvgRenderer.hpp"
#include "SvgTypes.hpp"

/**
 * @brief LabHappy SVG Path Renderer Demo
 * 
 * Demonstrates SVG symbol rendering on ESP32-P4 with LVGL
 * Tests: XOR Gate, AND Gate, Half Adder Circuit
 */
class LabHappySvgDemo : public LVWidget {
public:
    LabHappySvgDemo(LVWidget* parent = nullptr);
    virtual ~LabHappySvgDemo();
    
    /**
     * @brief Initialize demo screen
     */
    void init();
    
private:
    /**
     * @brief Create canvas for rendering
     */
    void createCanvas();
    
    /**
     * @brief Render test symbols
     */
    void renderTestSymbols();
    
    /**
     * @brief Create XOR Gate symbol
     */
    SvgRenderer::SvgSymbol createXorGateSymbol();
    
    /**
     * @brief Create AND Gate symbol
     */
    SvgRenderer::SvgSymbol createAndGateSymbol();
    
    LVCanvas* m_canvas;
    LVLabel* m_titleLabel;
    SvgRenderer::SvgRenderer* m_renderer;
    
    uint8_t* m_canvasBuffer;
    static const uint16_t CANVAS_WIDTH = 800;
    static const uint16_t CANVAS_HEIGHT = 480;
};
