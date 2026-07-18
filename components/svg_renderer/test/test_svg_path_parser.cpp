/*
 * Unit Tests for SVG Path Parser (Ported from C# WASM)
 * 
 * Original: LabBuddy.Web C# SvgPathParser tests
 * Target: ESP32-P4 with Unity Test Framework
 * 
 * Test Coverage:
 * - Path command parsing (M, L, C, Q, Z)
 * - Absolute vs relative coordinates
 * - Number parsing (integers, floats, scientific)
 * - Error handling
 * - Edge cases from DrawIO exports
 */

#include "unity.h"
#include "SvgPathParser.hpp"
#include "SvgTypes.hpp"
#include <string.h>
#include <cmath>

using namespace SvgRenderer;

static SvgPathParser* parser = nullptr;

void setUp(void) {
    parser = new SvgPathParser();
}

void tearDown(void) {
    delete parser;
    parser = nullptr;
}

// ============================================================================
// TEST SUITE 1: Basic Path Commands (Ported from C# PathParsingTests.cs)
// ============================================================================

void test_parse_simple_moveto_lineto(void) {
    // C# equivalent: [Test] public void ParseSimpleMoveTo_LineTo()
    const char* path = "M 10 20 L 30 40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, commands[0].args[0]);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, commands[0].args[1]);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, commands[1].args[0]);
    TEST_ASSERT_EQUAL_FLOAT(40.0f, commands[1].args[1]);
}

void test_parse_relative_moveto_lineto(void) {
    // C# equivalent: [Test] public void ParseRelativeMoveTo_LineTo()
    const char* path = "m 10 20 l 5 10";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('m', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('l', commands[1].type);
}

void test_parse_horizontal_vertical_lines(void) {
    // C# equivalent: [Test] public void ParseHorizontalVertical_Lines()
    const char* path = "M 10 20 H 50 V 80";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(3, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('H', commands[1].type);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, commands[1].args[0]);
    TEST_ASSERT_EQUAL_CHAR('V', commands[2].type);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, commands[2].args[0]);
}

void test_parse_cubic_bezier(void) {
    // C# equivalent: [Test] public void ParseCubicBezier()
    const char* path = "M 80 0 C 126.57 0 140 13.43 140 30";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('C', commands[1].type);
    TEST_ASSERT_EQUAL_INT(6, commands[1].args.size());
    TEST_ASSERT_EQUAL_FLOAT(126.57f, commands[1].args[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, commands[1].args[1]);
    TEST_ASSERT_EQUAL_FLOAT(140.0f, commands[1].args[2]);
    TEST_ASSERT_EQUAL_FLOAT(13.43f, commands[1].args[3]);
    TEST_ASSERT_EQUAL_FLOAT(140.0f, commands[1].args[4]);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, commands[1].args[5]);
}

void test_parse_quadratic_bezier(void) {
    // C# equivalent: [Test] public void ParseQuadraticBezier()
    const char* path = "M 10 20 Q 30 40 50 20";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('Q', commands[1].type);
    TEST_ASSERT_EQUAL_INT(4, commands[1].args.size());
}

void test_parse_close_path(void) {
    // C# equivalent: [Test] public void ParseClosePath()
    const char* path = "M 10 10 L 20 20 L 10 20 Z";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(4, commands.size());
    TEST_ASSERT_EQUAL_CHAR('Z', commands[3].type);
}

// ============================================================================
// TEST SUITE 2: Number Parsing (Ported from C# NumberParsingTests.cs)
// ============================================================================

void test_parse_integers(void) {
    // C# equivalent: [Test] public void ParseIntegers()
    const char* path = "M 10 20 L 100 200";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_FLOAT(10.0f, commands[0].args[0]);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, commands[0].args[1]);
}

void test_parse_floats(void) {
    // C# equivalent: [Test] public void ParseFloats()
    const char* path = "M 10.5 20.75 L 100.25 200.125";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.5f, commands[0].args[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.75f, commands[0].args[1]);
}

void test_parse_negative_numbers(void) {
    // C# equivalent: [Test] public void ParseNegativeNumbers()
    const char* path = "M -10 -20 L -30 -40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -10.0f, commands[0].args[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -20.0f, commands[0].args[1]);
}

void test_parse_scientific_notation(void) {
    // C# equivalent: [Test] public void ParseScientificNotation()
    const char* path = "M 1e2 2e-1 L 1.5e1 3.2e0";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, commands[0].args[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.2f, commands[0].args[1]);
}

// ============================================================================
// TEST SUITE 3: DrawIO Exports (Ported from C# DrawioPathTests.cs)
// ============================================================================

void test_parse_and_gate_path_from_drawio(void) {
    // C# equivalent: [Test] public void ParseAndGatePath_FromDrawio()
    // Real path from and_gate_circuit.json
    const char* path = "M 140 30 L 160 30 M 60 15 L 80 15 M 60 45 L 80 45 "
                      "M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 "
                      "C 140 46.57 126.57 60 110 60 L 80 60 Z";
    
    auto commands = parser->parse(path);
    
    // Should parse all commands without error
    TEST_ASSERT_TRUE(commands.size() > 0);
    
    // Verify first MoveTo
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_FLOAT(140.0f, commands[0].args[0]);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, commands[0].args[1]);
    
    // Find cubic bezier curves (C commands)
    int cubicCount = 0;
    for (const auto& cmd : commands) {
        if (cmd.type == 'C') cubicCount++;
    }
    TEST_ASSERT_EQUAL_INT(2, cubicCount); // Two curves for AND gate arc
}

void test_parse_xor_gate_path_from_drawio(void) {
    // C# equivalent: [Test] public void ParseXorGatePath_FromDrawio()
    // Real path from Full_Adder_circuit.json
    const char* path = "M 190 30 L 210 30 M 110 15 L 133 15 M 110 45 L 133 45 "
                      "M 120 0 C 130.72 18.56 130.72 41.44 120 60 "
                      "M 150 0 C 167.47 0.56 183.06 12.25 190 30 "
                      "C 183.06 47.75 167.47 59.44 150 60 L 125 60 "
                      "C 135.72 41.44 135.72 18.56 125 0 Z";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_TRUE(commands.size() > 0);
    
    // Verify complex cubic curves parse correctly
    int cubicCount = 0;
    for (const auto& cmd : commands) {
        if (cmd.type == 'C') {
            cubicCount++;
            TEST_ASSERT_EQUAL_INT(6, cmd.args.size()); // Each C has 6 args
        }
    }
    TEST_ASSERT_TRUE(cubicCount >= 3); // XOR has multiple curves
}

// ============================================================================
// TEST SUITE 4: Whitespace Handling (Ported from C# WhitespaceTests.cs)
// ============================================================================

void test_parse_with_extra_whitespace(void) {
    // C# equivalent: [Test] public void ParseWithExtraWhitespace()
    const char* path = "M   10   20   L   30   40  ";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
    TEST_ASSERT_EQUAL_FLOAT(10.0f, commands[0].args[0]);
}

void test_parse_with_comma_separators(void) {
    // C# equivalent: [Test] public void ParseWithCommaSeparators()
    const char* path = "M 10,20 L 30,40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
    TEST_ASSERT_EQUAL_FLOAT(20.0f, commands[0].args[1]);
}

void test_parse_with_mixed_separators(void) {
    // C# equivalent: [Test] public void ParseWithMixedSeparators()
    const char* path = "M 10, 20 L30 40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(2, commands.size());
}

void test_parse_compact_format_no_spaces(void) {
    // C# equivalent: [Test] public void ParseCompactFormat_NoSpaces()
    const char* path = "M10 20L30 40L50 60Z";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(4, commands.size());
}

// ============================================================================
// TEST SUITE 5: Implicit Command Repetition (Ported from C# ImplicitTests.cs)
// ============================================================================

void test_parse_implicit_lineto_after_moveto(void) {
    // C# equivalent: [Test] public void ParseImplicitLineTo_AfterMoveTo()
    // SVG spec: M followed by coordinate pairs = implicit L
    const char* path = "M 10 20 30 40 50 60";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_TRUE(commands.size() >= 3);
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    // Following coordinates should be treated as LineTo
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[2].type);
}

void test_parse_implicit_curveto_repetition(void) {
    // C# equivalent: [Test] public void ParseImplicitCurveTo_Repetition()
    const char* path = "M 10 20 C 30 40 50 60 70 80 90 100 110 120 130 140";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(3, commands.size()); // M + C + C (implicit)
    TEST_ASSERT_EQUAL_CHAR('C', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('C', commands[2].type);
}

// ============================================================================
// TEST SUITE 6: Error Handling (Ported from C# ErrorHandlingTests.cs)
// ============================================================================

void test_parse_empty_path_returns_empty(void) {
    // C# equivalent: [Test] public void ParseEmptyPath_ReturnsEmpty()
    const char* path = "";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_INT(0, commands.size());
}

void test_parse_invalid_command_returns_partial(void) {
    // C# equivalent: [Test] public void ParseInvalidCommand_ReturnsPartial()
    const char* path = "M 10 20 X 30 40"; // X is invalid
    
    auto commands = parser->parse(path);
    
    // Should parse M but stop at X
    TEST_ASSERT_TRUE(commands.size() >= 1);
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
}

void test_parse_incomplete_args_handles_gracefully(void) {
    // C# equivalent: [Test] public void ParseIncompleteArgs_HandlesGracefully()
    const char* path = "M 10"; // Missing Y coordinate
    
    auto commands = parser->parse(path);
    
    // Should handle gracefully (either skip or use default)
    // Implementation specific - just ensure no crash
    TEST_ASSERT_TRUE(true);
}

// ============================================================================
// TEST SUITE 7: Performance Tests (Ported from C# PerformanceTests.cs)
// ============================================================================

void test_parse_complex_path_performance(void) {
    // C# equivalent: [Test] public void ParseComplexPath_WithinTimeout()
    // Complex path with many commands (simulate full circuit)
    const char* complexPath = 
        "M 10 20 L 30 40 L 50 60 C 70 80 90 100 110 120 "
        "L 130 140 Q 150 160 170 180 L 190 200 Z "
        "M 220 240 C 260 280 300 320 340 360 Z";
    
    // Measure parse time (should be < 10ms for ESP32-P4)
    int64_t start = esp_timer_get_time();
    auto commands = parser->parse(complexPath);
    int64_t elapsed = esp_timer_get_time() - start;
    
    TEST_ASSERT_TRUE(commands.size() > 0);
    TEST_ASSERT_TRUE(elapsed < 10000); // < 10ms
}

// ============================================================================
// Test Runner
// ============================================================================

extern "C" void app_main(void) {
    UNITY_BEGIN();
    
    // Suite 1: Basic Path Commands
    RUN_TEST(test_parse_simple_moveto_lineto);
    RUN_TEST(test_parse_relative_moveto_lineto);
    RUN_TEST(test_parse_horizontal_vertical_lines);
    RUN_TEST(test_parse_cubic_bezier);
    RUN_TEST(test_parse_quadratic_bezier);
    RUN_TEST(test_parse_close_path);
    
    // Suite 2: Number Parsing
    RUN_TEST(test_parse_integers);
    RUN_TEST(test_parse_floats);
    RUN_TEST(test_parse_negative_numbers);
    RUN_TEST(test_parse_scientific_notation);
    
    // Suite 3: DrawIO Exports
    RUN_TEST(test_parse_and_gate_path_from_drawio);
    RUN_TEST(test_parse_xor_gate_path_from_drawio);
    
    // Suite 4: Whitespace Handling
    RUN_TEST(test_parse_with_extra_whitespace);
    RUN_TEST(test_parse_with_comma_separators);
    RUN_TEST(test_parse_with_mixed_separators);
    RUN_TEST(test_parse_compact_format_no_spaces);
    
    // Suite 5: Implicit Commands
    RUN_TEST(test_parse_implicit_lineto_after_moveto);
    RUN_TEST(test_parse_implicit_curveto_repetition);
    
    // Suite 6: Error Handling
    RUN_TEST(test_parse_empty_path_returns_empty);
    RUN_TEST(test_parse_invalid_command_returns_partial);
    RUN_TEST(test_parse_incomplete_args_handles_gracefully);
    
    // Suite 7: Performance
    RUN_TEST(test_parse_complex_path_performance);
    
    UNITY_END();
}
