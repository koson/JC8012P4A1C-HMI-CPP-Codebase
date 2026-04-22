#include "LabHappySvgDemo.hpp"
#include <cstdlib>

// XOR Gate path from Inkscape (IEEE symbol)
static const char* XOR_GATE_PATH = 
    "M 24.65625,70 H 45 C 65,70 75,50 75,50 75,50 70,30 45,30 H 24.65625 "
    "c 1.789926,3.094188 3.151364,6.453471 4.0625,10 C 29.54012,43.197121 30,46.545819 30,50 "
    "c 0,3.454181 -0.45988,6.802879 -1.28125,10 -0.911136,3.546529 -2.272574,6.905812 -4.0625,10 z "
    "M 18.65625,70 C 22.629549,64.322431 25,57.455703 25,50 25,42.544297 22.629549,35.677569 18.65625,30";

// AND Gate path from Inkscape (IEEE symbol)
static const char* AND_GATE_PATH = 
    "m 25,70 h 20 c 20,0 30,-20 30,-20 0,0 -5,-20 -30,-20 H 25 Z";

LabHappySvgDemo::LabHappySvgDemo(lv_obj_t* parent)
    : LVWidget(parent)
    , m_canvas(nullptr)
    , m_titleLabel(nullptr)
    , m_renderer(nullptr)
    , m_canvasBuffer(nullptr) {
}

LabHappySvgDemo::~LabHappySvgDemo() {
    if (m_renderer) {
        delete m_renderer;
    }
    if (m_canvas) {
        delete m_canvas;
    }
    if (m_titleLabel) {
        delete m_titleLabel;
    }
    if (m_canvasBuffer) {
        free(m_canvasBuffer);
    }
}

void LabHappySvgDemo::init() {
    // Set background color
    setBackgroundColor(LVColor::White);
    setSize(1024, 600);
    
    // Create title label
    m_titleLabel = new LVLabel(m_obj);
    m_titleLabel->setText("LabHappy SVG Path Renderer Demo");
    m_titleLabel->setPosition(20, 20);
    m_titleLabel->setTextColor(LVColor::Black);
    
    // Create canvas
    createCanvas();
    
    // Render test symbols
    renderTestSymbols();
}

void LabHappySvgDemo::createCanvas() {
    // Allocate canvas buffer (RGB565 format: 2 bytes per pixel)
    size_t bufferSize = CANVAS_WIDTH * CANVAS_HEIGHT * 2;
    m_canvasBuffer = (uint8_t*)malloc(bufferSize);
    
    if (!m_canvasBuffer) {
        // Handle allocation failure
        return;
    }
    
    // Create canvas
    m_canvas = new LVCanvas(
        m_obj,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        LV_COLOR_FORMAT_RGB565,
        m_canvasBuffer
    );
    
    // Position canvas
    m_canvas->setPosition(112, 60);  // Center on 1024x600 display
    
    // Clear canvas (white background)
    m_canvas->fill(LVColor::White);
    
    // Create SVG renderer
    m_renderer = new SvgRenderer::SvgRenderer(m_canvas);
    m_renderer->setBezierQuality(20, 15);  // High quality curves
}

void LabHappySvgDemo::renderTestSymbols() {
    if (!m_renderer) return;
    
    // Create symbol definitions
    auto xorGate = createXorGateSymbol();
    auto andGate = createAndGateSymbol();
    
    // Render XOR Gate at (100, 100)
    m_renderer->renderSymbol(
        xorGate,
        100, 100,
        SvgRenderer::Color::Black(),
        2  // strokeWidth
    );
    
    // Render AND Gate at (100, 250)
    m_renderer->renderSymbol(
        andGate,
        100, 250,
        SvgRenderer::Color::Black(),
        2  // strokeWidth
    );
    
    // Render scaled XOR Gate at (300, 100)
    m_renderer->renderSymbol(
        xorGate,
        300, 100,
        SvgRenderer::Color::Blue(),
        3,    // strokeWidth
        1.5f  // scale
    );
    
    // Render scaled AND Gate at (300, 250)
    m_renderer->renderSymbol(
        andGate,
        300, 250,
        SvgRenderer::Color::Red(),
        3,    // strokeWidth
        1.5f  // scale
    );
}

SvgRenderer::SvgSymbol LabHappySvgDemo::createXorGateSymbol() {
    SvgRenderer::SvgSymbol symbol;
    symbol.id = "Xor_Gate";
    symbol.pathData = XOR_GATE_PATH;
    symbol.viewBox = SvgRenderer::ViewBox(18, 30, 57, 40);
    symbol.scale = 1.0f;
    symbol.rotation = 0.0f;
    return symbol;
}

SvgRenderer::SvgSymbol LabHappySvgDemo::createAndGateSymbol() {
    SvgRenderer::SvgSymbol symbol;
    symbol.id = "And_Gate";
    symbol.pathData = AND_GATE_PATH;
    symbol.viewBox = SvgRenderer::ViewBox(25, 30, 50, 40);
    symbol.scale = 1.0f;
    symbol.rotation = 0.0f;
    return symbol;
}
