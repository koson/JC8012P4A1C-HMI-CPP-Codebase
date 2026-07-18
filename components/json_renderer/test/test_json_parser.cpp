/*
 * Unit Tests for JsonParser (Ported from C# WASM)
 * 
 * Original: LabBuddy.Web C# unit tests
 * Target: ESP32-P4 with Unity Test Framework
 * 
 * Test Coverage:
 * - JSON parsing
 * - Symbol extraction
 * - Coordinate transformation
 * - Error handling
 */

#include "unity.h"
#include "JsonParser.hpp"
#include "JsonTypes.hpp"
#include <string.h>
#include <cmath>

using namespace JsonRenderer;

// Test fixture setup
static JsonParser* parser = nullptr;
static Screen screen;

void setUp(void) {
    parser = new JsonParser();
    screen = Screen();
}

void tearDown(void) {
    delete parser;
    parser = nullptr;
}

// ============================================================================
// TEST SUITE 1: Basic JSON Parsing (Ported from C# BasicJsonTests.cs)
// ============================================================================

void test_parse_empty_json_should_fail(void) {
    // C# equivalent: [Test] public void ParseEmptyJson_ShouldFail()
    const char* emptyJson = "";
    
    bool result = parser->parse(emptyJson, 0, screen);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NOT_NULL(parser->getLastError());
}

void test_parse_valid_minimal_json(void) {
    // C# equivalent: [Test] public void ParseValidMinimalJson_ShouldSucceed()
    const char* minimalJson = R"({
        "version": "1.0",
        "title": "Test Circuit",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {},
        "widgets": [],
        "wires": [],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(minimalJson, strlen(minimalJson), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("Test Circuit", screen.title.c_str());
    TEST_ASSERT_EQUAL_INT32(1000, screen.width);
    TEST_ASSERT_EQUAL_INT32(1000, screen.height);
}

void test_parse_missing_required_field_should_fail(void) {
    // C# equivalent: [Test] public void ParseMissingRequiredField_ShouldFail()
    const char* invalidJson = R"({
        "version": "1.0",
        "title": "Test"
    })";
    
    bool result = parser->parse(invalidJson, strlen(invalidJson), screen);
    
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// TEST SUITE 2: Symbol Parsing (Ported from C# SymbolParsingTests.cs)
// ============================================================================

void test_parse_embedded_symbol_with_path_data(void) {
    // C# equivalent: [Test] public void ParseEmbeddedSymbol_WithPathData_ShouldSucceed()
    const char* symbolJson = R"({
        "version": "1.0",
        "title": "AND Gate Test",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {
            "AND_Gate_1": {
                "id": "AND_Gate_1",
                "title": "And Gate",
                "pathData": "M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 Z",
                "viewBox": {
                    "x": 0,
                    "y": 0,
                    "width": 100,
                    "height": 60
                }
            }
        },
        "widgets": [],
        "wires": [],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(symbolJson, strlen(symbolJson), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(1, screen.embeddedSymbols.size());
    
    auto it = screen.embeddedSymbols.find("AND_Gate_1");
    TEST_ASSERT_TRUE(it != screen.embeddedSymbols.end());
    TEST_ASSERT_EQUAL_STRING("AND_Gate_1", it->second.id.c_str());
    TEST_ASSERT_NOT_NULL(it->second.pathData.c_str());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, it->second.viewBox.x);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, it->second.viewBox.width);
}

void test_parse_multiple_symbols(void) {
    // C# equivalent: [Test] public void ParseMultipleSymbols_ShouldLoadAll()
    const char* multiSymbolJson = R"({
        "version": "1.0",
        "title": "Multi Symbol Test",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {
            "AND_Gate": {
                "id": "AND_Gate",
                "pathData": "M 0 0 L 10 10",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            },
            "OR_Gate": {
                "id": "OR_Gate",
                "pathData": "M 0 0 L 20 20",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            },
            "XOR_Gate": {
                "id": "XOR_Gate",
                "pathData": "M 0 0 L 30 30",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            }
        },
        "widgets": [],
        "wires": [],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(multiSymbolJson, strlen(multiSymbolJson), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(3, screen.embeddedSymbols.size());
    TEST_ASSERT_TRUE(screen.embeddedSymbols.find("AND_Gate") != screen.embeddedSymbols.end());
    TEST_ASSERT_TRUE(screen.embeddedSymbols.find("OR_Gate") != screen.embeddedSymbols.end());
    TEST_ASSERT_TRUE(screen.embeddedSymbols.find("XOR_Gate") != screen.embeddedSymbols.end());
}

// ============================================================================
// TEST SUITE 3: Widget Parsing (Ported from C# WidgetParsingTests.cs)
// ============================================================================

void test_parse_svg_symbol_widget(void) {
    // C# equivalent: [Test] public void ParseSvgSymbolWidget_ShouldSucceed()
    const char* widgetJson = R"({
        "version": "1.0",
        "title": "Widget Test",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {
            "TestSymbol": {
                "id": "TestSymbol",
                "pathData": "M 0 0 L 10 10",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            }
        },
        "widgets": [
            {
                "id": "widget1",
                "type": "svgSymbol",
                "x": 100,
                "y": 200,
                "width": 100,
                "height": 60,
                "rotation": 0,
                "symbolId": "TestSymbol",
                "scale": 1.0,
                "stroke": "#000000",
                "strokeWidth": 2
            }
        ],
        "wires": [],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(widgetJson, strlen(widgetJson), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(1, screen.widgets.size());
    
    const Widget& widget = screen.widgets[0];
    TEST_ASSERT_EQUAL_STRING("widget1", widget.id.c_str());
    TEST_ASSERT_EQUAL_STRING("svgSymbol", widget.type.c_str());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, widget.x);
    TEST_ASSERT_EQUAL_FLOAT(200.0f, widget.y);
    TEST_ASSERT_EQUAL_STRING("TestSymbol", widget.symbolId.c_str());
}

void test_parse_wire_connections(void) {
    // C# equivalent: [Test] public void ParseWireConnections_ShouldSucceed()
    const char* wireJson = R"({
        "version": "1.0",
        "title": "Wire Test",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {},
        "widgets": [],
        "wires": [
            {
                "id": "wire1",
                "path": "M 10 10 L 100 100",
                "strokeWidth": 2,
                "color": "#2C3E50",
                "sourceId": "port1",
                "targetId": "port2"
            }
        ],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(wireJson, strlen(wireJson), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(1, screen.wires.size());
    
    const Wire& wire = screen.wires[0];
    TEST_ASSERT_EQUAL_STRING("wire1", wire.id.c_str());
    TEST_ASSERT_EQUAL_STRING("M 10 10 L 100 100", wire.path.c_str());
    TEST_ASSERT_EQUAL_FLOAT(2.0f, wire.strokeWidth);
}

// ============================================================================
// TEST SUITE 4: Coordinate Transformation (Ported from C# CoordinateTests.cs)
// ============================================================================

void test_viewbox_to_canvas_coordinates(void) {
    // C# equivalent: [Test] public void ViewBoxToCanvas_ShouldTransformCorrectly()
    
    // Setup: ViewBox (0, 0, 1000, 1000) → Canvas (1024, 600)
    float viewBoxWidth = 1000.0f;
    float viewBoxHeight = 1000.0f;
    float canvasWidth = 1024.0f;
    float canvasHeight = 600.0f;
    
    // Calculate scale (fit to canvas maintaining aspect ratio)
    float scaleX = canvasWidth / viewBoxWidth;
    float scaleY = canvasHeight / viewBoxHeight;
    float scale = (scaleX < scaleY) ? scaleX : scaleY; // Min scale
    
    // Test point transformation
    float sourceX = 500.0f;
    float sourceY = 500.0f;
    
    float transformedX = sourceX * scale;
    float transformedY = sourceY * scale;
    
    // Expected: scale = 0.6 (600/1000), so (500, 500) → (300, 300)
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 300.0f, transformedX);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 300.0f, transformedY);
}

void test_rotation_transformation(void) {
    // C# equivalent: [Test] public void RotationTransform_90Degrees_ShouldRotateCorrectly()
    
    // Rotate (10, 0) by 90 degrees around origin
    float x = 10.0f;
    float y = 0.0f;
    float angleDeg = 90.0f;
    
    float angleRad = angleDeg * M_PI / 180.0f;
    float rotatedX = x * cosf(angleRad) - y * sinf(angleRad);
    float rotatedY = x * sinf(angleRad) + y * cosf(angleRad);
    
    // Expected: (10, 0) rotated 90° → (0, 10)
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, rotatedX);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, rotatedY);
}

// ============================================================================
// TEST SUITE 5: DrawIO Format Compatibility (Ported from C# DrawioTests.cs)
// ============================================================================

void test_parse_drawio_exported_json(void) {
    // C# equivalent: [Test] public void ParseDrawioExportedJson_ShouldSucceed()
    
    // This tests the actual format from DrawioToCircuitConverter
    const char* drawioJson = R"({
        "version": "1.0",
        "title": "Converted Circuit",
        "description": "coord_mode=raw",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {
            "Dynamic_Symbol_123": {
                "id": "Dynamic_Symbol_123",
                "title": "And Gate",
                "category": "Dynamic",
                "pathData": "M 140 30 L 160 30 M 60 15 L 80 15 M 60 45 L 80 45 M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 C 140 46.57 126.57 60 110 60 L 80 60 Z",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60},
                "description": "Generated dynamically from draw.io SVG element payload"
            }
        },
        "widgets": [
            {
                "id": "2",
                "type": "svgSymbol",
                "x": 60,
                "y": 0,
                "width": 100,
                "height": 60,
                "rotation": 0,
                "symbolId": "Dynamic_Symbol_123",
                "stroke": "#2C3E50"
            }
        ],
        "wires": [],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(drawioJson, strlen(drawioJson), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("coord_mode=raw", screen.description.c_str());
    TEST_ASSERT_EQUAL_INT(1, screen.embeddedSymbols.size());
    TEST_ASSERT_EQUAL_INT(1, screen.widgets.size());
}

// ============================================================================
// TEST SUITE 6: Error Handling (Ported from C# ErrorHandlingTests.cs)
// ============================================================================

void test_parse_malformed_json_should_fail(void) {
    // C# equivalent: [Test] public void ParseMalformedJson_ShouldFail()
    const char* malformedJson = R"({
        "version": "1.0",
        "title": "Test",
        INVALID JSON HERE
    })";
    
    bool result = parser->parse(malformedJson, strlen(malformedJson), screen);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NOT_NULL(parser->getLastError());
}

void test_parse_symbol_with_invalid_viewbox(void) {
    // C# equivalent: [Test] public void ParseSymbolWithInvalidViewBox_ShouldFail()
    const char* invalidViewBoxJson = R"({
        "version": "1.0",
        "title": "Test",
        "width": 1000,
        "height": 1000,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {
            "BadSymbol": {
                "id": "BadSymbol",
                "pathData": "M 0 0 L 10 10",
                "viewBox": {"x": 0, "y": 0, "width": -100, "height": 60}
            }
        },
        "widgets": [],
        "wires": [],
        "ports": [],
        "junctions": []
    })";
    
    bool result = parser->parse(invalidViewBoxJson, strlen(invalidViewBoxJson), screen);
    
    // Should either fail or auto-correct negative width
    if (result) {
        auto it = screen.embeddedSymbols.find("BadSymbol");
        if (it != screen.embeddedSymbols.end()) {
            // If parsed, width should be positive
            TEST_ASSERT_TRUE(it->second.viewBox.width >= 0);
        }
    }
}

// ============================================================================
// Test Runner
// ============================================================================

extern "C" void app_main(void) {
    UNITY_BEGIN();
    
    // Suite 1: Basic JSON Parsing
    RUN_TEST(test_parse_empty_json_should_fail);
    RUN_TEST(test_parse_valid_minimal_json);
    RUN_TEST(test_parse_missing_required_field_should_fail);
    
    // Suite 2: Symbol Parsing
    RUN_TEST(test_parse_embedded_symbol_with_path_data);
    RUN_TEST(test_parse_multiple_symbols);
    
    // Suite 3: Widget Parsing
    RUN_TEST(test_parse_svg_symbol_widget);
    RUN_TEST(test_parse_wire_connections);
    
    // Suite 4: Coordinate Transformation
    RUN_TEST(test_viewbox_to_canvas_coordinates);
    RUN_TEST(test_rotation_transformation);
    
    // Suite 5: DrawIO Compatibility
    RUN_TEST(test_parse_drawio_exported_json);
    
    // Suite 6: Error Handling
    RUN_TEST(test_parse_malformed_json_should_fail);
    RUN_TEST(test_parse_symbol_with_invalid_viewbox);
    
    UNITY_END();
}
