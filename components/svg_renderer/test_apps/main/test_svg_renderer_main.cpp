/*
 * Unit Tests for SvgPathParser
 * 
 * Target: ESP32-P4 with Unity Test Framework
 * 
 * Test Coverage:
 * - SVG path command parsing (M, L, H, V, C, Q, Z)
 * - Absolute and relative coordinates
 * - Bezier curves
 * - Real DrawIO path data
 */

#include "unity.h"
#include "unity_test_runner.h"
#include "SvgPathParser.hpp"
#include "SvgTypes.hpp"
#include <string.h>
#include <cmath>

using namespace SvgRenderer;

// Test fixture
static SvgPathParser* parser = nullptr;

void setUp(void) {
    parser = new SvgPathParser();
}

void tearDown(void) {
    delete parser;
    parser = nullptr;
}

// Helper to compare floats with tolerance
static void assertFloatEqual(float expected, float actual, float tolerance = 0.01f) {
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected, actual);
}

// ============================================================================
// TEST SUITE 1: Basic Move and Line Commands
// ============================================================================

static void test_parse_simple_move_absolute(void) {
    const char* path = "M 10 20";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(1, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_UINT32(2, commands[0].args.size());
    assertFloatEqual(10.0f, commands[0].args[0]);
    assertFloatEqual(20.0f, commands[0].args[1]);
}

static void test_parse_simple_line_absolute(void) {
    const char* path = "M 0 0 L 100 50";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    assertFloatEqual(100.0f, commands[1].args[0]);
    assertFloatEqual(50.0f, commands[1].args[1]);
}

static void test_parse_horizontal_line(void) {
    const char* path = "M 10 20 H 50";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    // H command should be present or converted to L
    bool found_h_or_l = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'H' || cmd.type == 'L') {
            found_h_or_l = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_h_or_l);
}

static void test_parse_vertical_line(void) {
    const char* path = "M 10 20 V 80";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    // V command should be present or converted to L
    bool found_v_or_l = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'V' || cmd.type == 'L') {
            found_v_or_l = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_v_or_l);
}

static void test_parse_close_path(void) {
    const char* path = "M 0 0 L 10 10 Z";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(3, commands.size());
    TEST_ASSERT_EQUAL_CHAR('Z', commands[2].type);
}

// ============================================================================
// TEST SUITE 2: Relative Commands
// ============================================================================

static void test_parse_relative_move(void) {
    const char* path = "M 10 20 m 5 10";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    // Should have at least M and m (or converted)
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
}

static void test_parse_relative_line(void) {
    const char* path = "M 0 0 l 10 20";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
}

// ============================================================================
// TEST SUITE 3: Cubic Bezier Curves
// ============================================================================

static void test_parse_cubic_bezier_absolute(void) {
    const char* path = "M 0 0 C 10 20, 30 40, 50 60";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    
    // Find C command
    bool found_c = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'C') {
            found_c = true;
            TEST_ASSERT_EQUAL_UINT32(6, cmd.args.size()); // 3 points × 2 coords
            break;
        }
    }
    TEST_ASSERT_TRUE(found_c);
}

static void test_parse_cubic_bezier_relative(void) {
    const char* path = "M 0 0 c 10 20, 30 40, 50 60";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
}

// ============================================================================
// TEST SUITE 4: Quadratic Bezier Curves
// ============================================================================

static void test_parse_quadratic_bezier_absolute(void) {
    const char* path = "M 0 0 Q 10 20, 30 40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    
    // Find Q command
    bool found_q = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'Q') {
            found_q = true;
            TEST_ASSERT_EQUAL_UINT32(4, cmd.args.size()); // 2 points × 2 coords
            break;
        }
    }
    TEST_ASSERT_TRUE(found_q);
}

// ============================================================================
// TEST SUITE 5: Multiple Commands
// ============================================================================

static void test_parse_multiple_segments(void) {
    const char* path = "M 0 0 L 10 10 L 20 20 L 30 30";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(4, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[2].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[3].type);
}

static void test_parse_implicit_lineto(void) {
    // After M, multiple coordinate pairs should be treated as L
    const char* path = "M 0 0 10 10 20 20";
    
    auto commands = parser->parse(path);
    
    // Should have M followed by implicit L commands
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
}

// ============================================================================
// TEST SUITE 6: Real DrawIO Paths
// ============================================================================

static void test_parse_and_gate_path_from_drawio(void) {
    // Real AND gate path from DrawIO export
    const char* path = "M 140 30 L 160 30 M 60 15 L 80 15 M 60 45 L 80 45 M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 C 140 46.57 126.57 60 110 60 L 80 60 Z";
    
    auto commands = parser->parse(path);
    
    // Should successfully parse without errors
    TEST_ASSERT_GREATER_THAN_UINT32(0, commands.size());
    TEST_ASSERT_NULL(parser->getLastError()[0] ? parser->getLastError() : nullptr);
    
    // Verify first command is M
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    
    // Should contain cubic bezier curves (C)
    bool found_bezier = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'C') {
            found_bezier = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_bezier);
}

static void test_parse_or_gate_path(void) {
    // Simplified OR gate path
    const char* path = "M 80 0 Q 95 0, 110 15 Q 110 30, 110 45 Q 95 60, 80 60 Z";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_THAN_UINT32(0, commands.size());
    
    // Should contain quadratic bezier curves
    bool found_quadratic = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'Q') {
            found_quadratic = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_quadratic);
}

// ============================================================================
// TEST SUITE 7: Error Handling
// ============================================================================

static void test_parse_empty_string(void) {
    const char* path = "";
    
    auto commands = parser->parse(path);
    
    // Empty path should return empty vector
    TEST_ASSERT_EQUAL_UINT32(0, commands.size());
}

static void test_parse_invalid_command(void) {
    const char* path = "M 0 0 X 10 10"; // X is not valid SVG command
    
    auto commands = parser->parse(path);
    
    // Should handle invalid command gracefully
    // Either skip it or stop parsing
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, commands.size());
}

// ============================================================================
// TEST SUITE 8: Whitespace Handling
// ============================================================================

static void test_parse_comma_separated(void) {
    const char* path = "M 10,20 L 30,40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(2, commands.size());
    assertFloatEqual(10.0f, commands[0].args[0]);
    assertFloatEqual(20.0f, commands[0].args[1]);
}

static void test_parse_no_spaces(void) {
    const char* path = "M10,20L30,40";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(2, commands.size());
}

static void test_parse_extra_whitespace(void) {
    const char* path = "M   10   20   L   30   40  ";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_EQUAL_UINT32(2, commands.size());
}

// ============================================================================
// Main test runner
// ============================================================================

extern "C" void app_main(void)
{
    UNITY_BEGIN();
    
    // Basic commands
    RUN_TEST(test_parse_simple_move_absolute);
    RUN_TEST(test_parse_simple_line_absolute);
    RUN_TEST(test_parse_horizontal_line);
    RUN_TEST(test_parse_vertical_line);
    RUN_TEST(test_parse_close_path);
    
    // Relative commands
    RUN_TEST(test_parse_relative_move);
    RUN_TEST(test_parse_relative_line);
    
    // Bezier curves
    RUN_TEST(test_parse_cubic_bezier_absolute);
    RUN_TEST(test_parse_cubic_bezier_relative);
    RUN_TEST(test_parse_quadratic_bezier_absolute);
    
    // Multiple segments
    RUN_TEST(test_parse_multiple_segments);
    RUN_TEST(test_parse_implicit_lineto);
    
    // Real DrawIO paths
    RUN_TEST(test_parse_and_gate_path_from_drawio);
    RUN_TEST(test_parse_or_gate_path);
    
    // Error handling
    RUN_TEST(test_parse_empty_string);
    RUN_TEST(test_parse_invalid_command);
    
    // Whitespace
    RUN_TEST(test_parse_comma_separated);
    RUN_TEST(test_parse_no_spaces);
    RUN_TEST(test_parse_extra_whitespace);
    
    UNITY_END();
}
