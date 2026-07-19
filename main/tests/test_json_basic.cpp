/*
 * JSON Parser Basic Tests
 * 
 * Tests basic JSON parsing functionality for circuit files
 */

#include "unity.h"
#include "JsonParser.hpp"
#include "JsonTypes.hpp"
#include <string.h>

using namespace JsonRenderer;

// Test fixtures
static JsonParser* parser = nullptr;
static Screen screen;

void setUp_json(void) {
    parser = new JsonParser();
    screen = Screen();
}

void tearDown_json(void) {
    delete parser;
    parser = nullptr;
}

// ============================================================================
// Basic Parsing Tests
// ============================================================================

void test_json_parse_empty_should_fail(void) {
    setUp_json();
    
    const char* emptyJson = "";
    bool result = parser->parseString(emptyJson, screen);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NOT_NULL(parser->getLastError());
    
    tearDown_json();
}

void test_json_parse_minimal_valid(void) {
    setUp_json();
    
    const char* json = R"({
        "version": "1.0",
        "title": "Test Circuit",
        "width": 1000,
        "height": 800,
        "backgroundColor": "#FFFFFF"
    })";
    
    bool result = parser->parseString(json, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_STRING("Test Circuit", screen.title.c_str());
    TEST_ASSERT_EQUAL_INT32(1000, screen.width);
    TEST_ASSERT_EQUAL_INT32(800, screen.height);
    TEST_ASSERT_EQUAL_STRING("#FFFFFF", screen.backgroundColor.c_str());
    
    tearDown_json();
}

void test_json_parse_invalid_syntax_should_fail(void) {
    setUp_json();
    
    const char* invalidJson = "{ this is not valid JSON }";
    bool result = parser->parseString(invalidJson, screen);
    
    TEST_ASSERT_FALSE(result);
    
    tearDown_json();
}

// ============================================================================
// Symbol Parsing Tests
// ============================================================================

void test_json_parse_single_symbol(void) {
    setUp_json();
    
    const char* json = R"({
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
    
    bool result = parser->parseString(json, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(1, screen.embeddedSymbols.size());
    
    auto it = screen.embeddedSymbols.find("and_gate");
    TEST_ASSERT_TRUE(it != screen.embeddedSymbols.end());
    TEST_ASSERT_EQUAL_STRING("and_gate", it->second.id.c_str());
    TEST_ASSERT_EQUAL_STRING("AND Gate", it->second.title.c_str());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, it->second.viewBox.width);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, it->second.viewBox.height);
    
    tearDown_json();
}

void test_json_parse_multiple_symbols(void) {
    setUp_json();
    
    const char* json = R"({
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
            },
            "gate3": {
                "id": "gate3",
                "pathData": "M 0 0",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            }
        }
    })";
    
    bool result = parser->parseString(json, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(3, screen.embeddedSymbols.size());
    TEST_ASSERT_TRUE(screen.embeddedSymbols.find("gate1") != screen.embeddedSymbols.end());
    TEST_ASSERT_TRUE(screen.embeddedSymbols.find("gate2") != screen.embeddedSymbols.end());
    TEST_ASSERT_TRUE(screen.embeddedSymbols.find("gate3") != screen.embeddedSymbols.end());
    
    tearDown_json();
}

// ============================================================================
// Widget Parsing Tests
// ============================================================================

void test_json_parse_widget_basic(void) {
    setUp_json();
    
    const char* json = R"({
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
                "scale": 1.5,
                "rotation": 0
            }
        ]
    })";
    
    bool result = parser->parseString(json, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(1, screen.widgets.size());
    TEST_ASSERT_EQUAL_STRING("svgSymbol", screen.widgets[0].type.c_str());
    TEST_ASSERT_EQUAL_STRING("test_symbol", screen.widgets[0].symbolId.c_str());
    TEST_ASSERT_EQUAL_FLOAT(100.0f, screen.widgets[0].x);
    TEST_ASSERT_EQUAL_FLOAT(200.0f, screen.widgets[0].y);
    TEST_ASSERT_EQUAL_FLOAT(1.5f, screen.widgets[0].scale);
    
    tearDown_json();
}

void test_json_parse_wires(void) {
    setUp_json();
    
    const char* json = R"({
        "version": "1.0",
        "title": "Wire Test",
        "width": 1000,
        "height": 800,
        "wires": [
            {
                "id": "wire1",
                "path": "M 0 0 L 100 100",
                "strokeWidth": 2
            },
            {
                "id": "wire2",
                "path": "M 50 50 L 150 150",
                "strokeWidth": 3
            }
        ]
    })";
    
    bool result = parser->parseString(json, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_UINT32(2, screen.wires.size());
    TEST_ASSERT_EQUAL_STRING("wire1", screen.wires[0].id.c_str());
    TEST_ASSERT_EQUAL_STRING("M 0 0 L 100 100", screen.wires[0].path.c_str());
    TEST_ASSERT_EQUAL_FLOAT(2.0f, screen.wires[0].strokeWidth);
    
    tearDown_json();
}

// ============================================================================
// Real-world Circuit Tests
// ============================================================================

void test_json_parse_and_gate_circuit(void) {
    setUp_json();
    
    // Simplified real DrawIO AND gate export
    const char* json = R"({
        "version": "1.0",
        "title": "AND Gate Circuit",
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
                "rotation": 0
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
        ]
    })";
    
    bool result = parser->parseString(json, screen);
    
    TEST_ASSERT_TRUE_MESSAGE(result, parser->getLastError());
    TEST_ASSERT_EQUAL_STRING("AND Gate Circuit", screen.title.c_str());
    TEST_ASSERT_EQUAL_UINT32(1, screen.embeddedSymbols.size());
    TEST_ASSERT_EQUAL_UINT32(1, screen.widgets.size());
    TEST_ASSERT_EQUAL_UINT32(1, screen.wires.size());
    TEST_ASSERT_EQUAL_UINT32(1, screen.ports.size());
    
    // Verify symbol has path data
    auto symbolIt = screen.embeddedSymbols.find("AND_Gate_1");
    TEST_ASSERT_TRUE(symbolIt != screen.embeddedSymbols.end());
    TEST_ASSERT_GREATER_THAN(0, strlen(symbolIt->second.pathData.c_str()));
    
    tearDown_json();
}
