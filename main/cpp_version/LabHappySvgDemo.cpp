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

// Simple test path (rectangle) - เพื่อทดสอบ
static const char* TEST_RECTANGLE_PATH = 
    "M 0,0 L 50,0 L 50,40 L 0,40 Z";

LabHappySvgDemo::LabHappySvgDemo(LVWidget* parent)
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
    setBackgroundColor(LVColor::White.raw());
    setSize(1024, 600);
    
    // Create title label
    m_titleLabel = new LVLabel(this);
    m_titleLabel->setText("LabHappy SVG Path Renderer Demo");
    m_titleLabel->setPos(20, 20);
    m_titleLabel->setTextColor(LVColor::Black.raw());
    
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
    
    // Create canvas (parent is lv_obj_t*)
    m_canvas = new LVCanvas(
        m_obj,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        LV_COLOR_FORMAT_RGB565,
        m_canvasBuffer
    );
    
    // Position canvas
    lv_obj_align(m_canvas->obj(), LV_ALIGN_TOP_LEFT, 112, 60);
    
    // Clear canvas (white background)
    m_canvas->fill(LVColor::White);
    
    // Create SVG renderer
    m_renderer = new SvgRenderer::SvgRenderer(m_canvas);
    m_renderer->setBezierQuality(30, 20);  // เพิ่มความละเอียด (30 segments สำหรับ cubic, 20 สำหรับ quadratic)
}

void LabHappySvgDemo::renderTestSymbols() {
    if (!m_renderer) return;
    
    // Test rectangle first - simple path
    SvgRenderer::SvgSymbol testRect;
    testRect.id = "Test_Rect";
    testRect.pathData = TEST_RECTANGLE_PATH;
    testRect.viewBox = SvgRenderer::ViewBox(0, 0, 50, 40);
    testRect.scale = 1.0f;
    testRect.rotation = 0.0f;
    
    // Render test rectangle at (400, 50)
    m_renderer->renderSymbol(
        testRect,
        400, 50,
        SvgRenderer::Color::Green(),
        3  // strokeWidth
    );
    
    // Create symbol definitions
    auto xorGate = createXorGateSymbol();
    auto andGate = createAndGateSymbol();
    
    // Render XOR Gate at (50, 50) - ห่างจากขอบ
    m_renderer->renderSymbol(
        xorGate,
        50, 50,
        SvgRenderer::Color::Black(),
        3  // strokeWidth - เพิ่มจาก 2 เป็น 3
    );
    
    // Render AND Gate at (50, 150)
    m_renderer->renderSymbol(
        andGate,
        50, 150,
        SvgRenderer::Color::Black(),
        3  // strokeWidth - เพิ่มจาก 2 เป็น 3
    );
    
    // Render scaled XOR Gate at (200, 50)
    m_renderer->renderSymbol(
        xorGate,
        200, 50,
        SvgRenderer::Color::Blue(),
        4,    // strokeWidth - เพิ่มจาก 3 เป็น 4
        2.0f  // scale - ใหญ่ขึ้น
    );
    
    // Render scaled AND Gate at (200, 200)
    m_renderer->renderSymbol(
        andGate,
        200, 200,
        SvgRenderer::Color::Red(),
        4,    // strokeWidth - เพิ่มจาก 3 เป็น 4
        2.0f  // scale - ใหญ่ขึ้น
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

// Export C function for MenuDemoApplication
extern "C" void create_labhappy_svg_demo() {
    auto* screen = lv_screen_active();
    auto* demo = new LabHappySvgDemo(nullptr);
    lv_obj_set_parent(demo->obj(), screen);
    demo->init();
}
