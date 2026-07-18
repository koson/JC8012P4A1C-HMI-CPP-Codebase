/*
 * Unit Tests for JsonParser
 * 
 * Target: ESP32-P4 with Unity Test Framework
 * 
 * Test Coverage:
 * - JSON parsing (string-based)
 * - Symbol extraction
 * - Widget parsing
 * - Error handling
 */

#include "unity.h"
#include "unity_test_runner.h"
#include "JsonParser.hpp"
#include "JsonTypes.hpp"
#include <string.h>

using namespace JsonRenderer;

// Test fixture
static JsonParser* parser = nullptr;
static Screen screen;

void setUp(void) {
    parser = new JsonParser();
    // Reset screen for each test
    screen = Screen();
}

void tearDown(void) {
    delete parser;
    parser = nullptr;
}

// ============================================================================
// TEST SUITE 1: Basic JSON Parsing
// ============================================================================

static void test_parse_empty_string_should_fail(void) {
    const char* emptyJson = "";
    
    bool result = parser->parseString(emptyJson, screen);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NOT_NULL(parser->getLastError());
}

static void test_parse_valid_minimal_json(void) {
    const char* minimalJson = R"({
        "version": "1.0",
        "title": "Test Circuit",
        "width": 1000,
        "height": 800,
        "backgroundColor": "#FFFFFF"
    })";
    
    bool result = parser->parseString(minimalJson, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_STRING("Test Circuit", screen.title.c_str());
    TEST_ASSERT_EQUAL_INT32(1000, screen.width);
    TEST_ASSERT_EQUAL_INT32(800, screen.height);
}

static void test_parse_invalid_json_should_fail(void) {
    const char* invalidJson = "{ invalid json }";
    
    bool result = parser->parseString(invalidJson, screen);
    
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// TEST SUITE 2: Symbol Parsing
// ============================================================================

static void test_parse_embedded_symbol_basic(void) {
    const char* symbolJson = R"({
        "version": "1.0",
        "title": "Symbol Test",
        "width": 1000,
        "height": 800,
        "backgroundColor": "#FFFFFF",
        "embeddedSymbols": {
            "and_gate": {
                "id": "and_gate",
                "title": "AND Gate",
                "pathData": "M 0 0 L 100 50 L 0 100 Z",
                "viewBox": {
                    "x": 0,
                    "y": 0,
                    "width": 100,
                    "height": 100
                }
            }
        }
    })";
    
    bool result = parser->parseString(symbolJson, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(1, screen.embeddedSymbols.size());
    
    auto it = screen.embeddedSymbols.find("and_gate");
    TEST_ASSERT_TRUE(it != screen.embeddedSymbols.end());
    TEST_ASSERT_EQUAL_STRING("and_gate", it->second.id.c_str());
    TEST_ASSERT_EQUAL_STRING("AND Gate", it->second.title.c_str());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, it->second.viewBox.width);
}

static void test_parse_multiple_symbols(void) {
    const char* multiSymbolJson = R"({
        "version": "1.0",
        "title": "Multi Symbol",
        "width": 1000,
        "height": 800,
        "embeddedSymbols": {
            "gate1": {
                "id": "gate1",
                "pathData": "M 0 0",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            },
            "gate2": {
                "id": "gate2",
                "pathData": "M 0 0",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            }
        }
    })";
    
    bool result = parser->parseString(multiSymbolJson, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(2, screen.embeddedSymbols.size());
}

// ============================================================================
// TEST SUITE 3: Widget Parsing
// ============================================================================

static void test_parse_svg_symbol_widget(void) {
    const char* widgetJson = R"({
        "version": "1.0",
        "title": "Widget Test",
        "width": 1000,
        "height": 800,
        "embeddedSymbols": {
            "test_symbol": {
                "id": "test_symbol",
                "pathData": "M 0 0 L 10 10",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            }
        },
        "widgets": [
            {
                "type": "svgSymbol",
                "symbolId": "test_symbol",
                "x": 100,
                "y": 200,
                "scale": 1.0,
                "rotation": 0
            }
        ]
    })";
    
    bool result = parser->parseString(widgetJson, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(1, screen.widgets.size());
    TEST_ASSERT_EQUAL_STRING("svgSymbol", screen.widgets[0].type.c_str());
    TEST_ASSERT_EQUAL_STRING("test_symbol", screen.widgets[0].symbolId.c_str());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, screen.widgets[0].x);
    TEST_ASSERT_EQUAL_FLOAT(200.0f, screen.widgets[0].y);
}

// ============================================================================
// TEST SUITE 4: Wire Parsing
// ============================================================================

static void test_parse_wires(void) {
    const char* wireJson = R"({
        "version": "1.0",
        "title": "Wire Test",
        "width": 1000,
        "height": 800,
        "wires": [
            {
                "id": "wire1",
                "path": "M 0 0 L 100 100",
                "strokeWidth": 2
            }
        ]
    })";
    
    bool result = parser->parseString(wireJson, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(1, screen.wires.size());
    TEST_ASSERT_EQUAL_STRING("wire1", screen.wires[0].id.c_str());
    TEST_ASSERT_EQUAL_STRING("M 0 0 L 100 100", screen.wires[0].path.c_str());
}

// ============================================================================
// TEST SUITE 5: Real Circuit JSON (from DrawIO export)
// ============================================================================

static void test_parse_drawio_and_gate(void) {
    // Simplified AND gate from DrawIO export
    const char* andGateJson = R"({
        "version": "1.0",
        "title": "AND Gate",
        "width": 1024,
        "height": 600,
        "backgroundColor": "#FFFFFF",
        "coord_mode": "raw",
        "embeddedSymbols": {
            "AND_Gate_1": {
                "id": "AND_Gate_1",
                "title": "And Gate",
                "pathData": "M 140 30 L 160 30 M 60 15 L 80 15 M 60 45 L 80 45 M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 C 140 46.57 126.57 60 110 60 L 80 60 Z",
                "viewBox": {
                    "x": 0,
                    "y": 0,
                    "width": 160,
                    "height": 60
                }
            }
        },
        "widgets": [
            {
                "type": "svgSymbol",
                "symbolId": "AND_Gate_1",
                "x": 120,
                "y": 80,
                "scale": 1.0,
                "rotation": 0,
                "flipHorizontal": false,
                "flipVertical": false
            }
        ],
        "wires": [
            {
                "id": "wire1",
                "path": "M 20 95 L 180 95",
                "strokeWidth": 2
            }
        ],
        "ports": [
            {
                "id": "in1",
                "x": 20,
                "y": 95,
                "type": "input",
                "radius": 5
            }
        ],
        "junctions": []
    })";
    
    bool result = parser->parseString(andGateJson, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_STRING("AND Gate", screen.title.c_str());
    TEST_ASSERT_EQUAL_UINT32(1, screen.embeddedSymbols.size());
    TEST_ASSERT_EQUAL_UINT32(1, screen.widgets.size());
    TEST_ASSERT_EQUAL_UINT32(1, screen.wires.size());
    TEST_ASSERT_EQUAL_UINT32(1, screen.ports.size());
    
    // Verify symbol content
    auto symbolIt = screen.embeddedSymbols.find("AND_Gate_1");
    TEST_ASSERT_TRUE(symbolIt != screen.embeddedSymbols.end());
    TEST_ASSERT_GREATER_THAN(0, strlen(symbolIt->second.pathData.c_str()));
}

// ============================================================================
// Main test runner
// ============================================================================

extern "C" void app_main(void)
{
    UNITY_BEGIN();
    
    // Basic parsing tests
    RUN_TEST(test_parse_empty_string_should_fail);
    RUN_TEST(test_parse_valid_minimal_json);
    RUN_TEST(test_parse_invalid_json_should_fail);
    
    // Symbol parsing tests
    RUN_TEST(test_parse_embedded_symbol_basic);
    RUN_TEST(test_parse_multiple_symbols);
    
    // Widget parsing tests
    RUN_TEST(test_parse_svg_symbol_widget);
    
    // Wire parsing tests
    RUN_TEST(test_parse_wires);
    
    // Real circuit test
    RUN_TEST(test_parse_drawio_and_gate);
    
    UNITY_END();
}
