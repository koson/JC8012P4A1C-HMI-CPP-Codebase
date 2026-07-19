/*
 * SVG Path Parser Basic Tests
 * 
 * Tests SVG path parsing functionality
 */

#include "unity.h"
#include "SvgPathParser.hpp"
#include "SvgTypes.hpp"
#include <string.h>
#include <cmath>

using namespace SvgRenderer;

// Test fixtures
static SvgPathParser* parser = nullptr;

void setUp_svg(void) {
    parser = new SvgPathParser();
}

void tearDown_svg(void) {
    delete parser;
    parser = nullptr;
}

// Helper to compare floats
static void assertFloatEqual(float expected, float actual, float tolerance = 0.01f) {
    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected, actual);
}

// ============================================================================
// Basic Command Tests
// ============================================================================

void test_svg_parse_move_absolute(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 10 20");
    
    TEST_ASSERT_EQUAL_UINT32(1, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_UINT32(2, commands[0].args.size());
    assertFloatEqual(10.0f, commands[0].args[0]);
    assertFloatEqual(20.0f, commands[0].args[1]);
    
    tearDown_svg();
}

void test_svg_parse_line_absolute(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 0 0 L 100 50");
    
    TEST_ASSERT_EQUAL_UINT32(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    assertFloatEqual(100.0f, commands[1].args[0]);
    assertFloatEqual(50.0f, commands[1].args[1]);
    
    tearDown_svg();
}

void test_svg_parse_horizontal_line(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 10 20 H 50");
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    
    // H command should be present or converted to L
    bool found = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'H' || cmd.type == 'L') {
            found = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found);
    
    tearDown_svg();
}

void test_svg_parse_vertical_line(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 10 20 V 80");
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    
    bool found = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'V' || cmd.type == 'L') {
            found = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found);
    
    tearDown_svg();
}

void test_svg_parse_close_path(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 0 0 L 10 10 Z");
    
    TEST_ASSERT_EQUAL_UINT32(3, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('Z', commands[2].type);
    
    tearDown_svg();
}

// ============================================================================
// Bezier Curve Tests
// ============================================================================

void test_svg_parse_cubic_bezier(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 0 0 C 10 20, 30 40, 50 60");
    
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
    
    tearDown_svg();
}

void test_svg_parse_quadratic_bezier(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 0 0 Q 10 20, 30 40");
    
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
    
    tearDown_svg();
}

// ============================================================================
// Multiple Segments Tests
// ============================================================================

void test_svg_parse_multiple_lines(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 0 0 L 10 10 L 20 20 L 30 30");
    
    TEST_ASSERT_EQUAL_UINT32(4, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[2].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[3].type);
    
    tearDown_svg();
}

void test_svg_parse_complex_path(void) {
    setUp_svg();
    
    auto commands = parser->parse("M 10 10 L 20 20 H 30 V 40 Z");
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(4, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    
    tearDown_svg();
}

// ============================================================================
// Real DrawIO Paths
// ============================================================================

void test_svg_parse_and_gate_path(void) {
    setUp_svg();
    
    // Real AND gate path from DrawIO
    const char* path = "M 140 30 L 160 30 M 60 15 L 80 15 M 60 45 L 80 45 "
                      "M 80 0 L 110 0 C 126.57 0 140 13.43 140 30 "
                      "C 140 46.57 126.57 60 110 60 L 80 60 Z";
    
    auto commands = parser->parse(path);
    
    // Should successfully parse
    TEST_ASSERT_GREATER_THAN_UINT32(0, commands.size());
    
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
    TEST_ASSERT_TRUE_MESSAGE(found_bezier, "AND gate should have bezier curves");
    
    tearDown_svg();
}

void test_svg_parse_or_gate_path(void) {
    setUp_svg();
    
    // Simplified OR gate with quadratic bezier
    const char* path = "M 80 0 Q 95 0, 110 15 Q 110 30, 110 45 Q 95 60, 80 60 Z";
    
    auto commands = parser->parse(path);
    
    TEST_ASSERT_GREATER_THAN_UINT32(0, commands.size());
    
    // Should contain quadratic bezier
    bool found_q = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'Q') {
            found_q = true;
            break;
        }
    }
    TEST_ASSERT_TRUE_MESSAGE(found_q, "OR gate should have quadratic bezier");
    
    tearDown_svg();
}

// ============================================================================
// Error Handling Tests
// ============================================================================

void test_svg_parse_empty_string(void) {
    setUp_svg();
    
    auto commands = parser->parse("");
    
    // Empty path should return empty vector
    TEST_ASSERT_EQUAL_UINT32(0, commands.size());
    
    tearDown_svg();
}

void test_svg_parse_whitespace_handling(void) {
    setUp_svg();
    
    // Test comma-separated
    auto commands1 = parser->parse("M 10,20 L 30,40");
    TEST_ASSERT_EQUAL_UINT32(2, commands1.size());
    
    // Test no spaces
    auto commands2 = parser->parse("M10,20L30,40");
    TEST_ASSERT_EQUAL_UINT32(2, commands2.size());
    
    // Test extra whitespace
    auto commands3 = parser->parse("M   10   20   L   30   40  ");
    TEST_ASSERT_EQUAL_UINT32(2, commands3.size());
    
    tearDown_svg();
}

// ============================================================================
// Relative Command Tests
// ============================================================================

void test_svg_parse_relative_commands(void) {
    setUp_svg();
    
    // Mix of absolute and relative
    auto commands = parser->parse("M 10 20 l 5 10 l 5 10");
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    
    tearDown_svg();
}

void test_svg_parse_implicit_lines_after_move(void) {
    setUp_svg();

    // Migrated from WASM geometry-path tests: SVG allows implicit L after M.
    auto commands = parser->parse("M 0,0 10,10 20,5");

    TEST_ASSERT_EQUAL_UINT32(3, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[2].type);
    assertFloatEqual(10.0f, commands[1].args[0]);
    assertFloatEqual(10.0f, commands[1].args[1]);
    assertFloatEqual(20.0f, commands[2].args[0]);
    assertFloatEqual(5.0f, commands[2].args[1]);

    tearDown_svg();
}

void test_svg_parse_repeated_relative_commands_without_retyping_letter(void) {
    setUp_svg();

    // Regression guard: command repetition should work when command letter is omitted.
    auto commands = parser->parse("M 10,10 h 5 7 v 3 2");

    TEST_ASSERT_EQUAL_UINT32(5, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('h', commands[1].type);
    TEST_ASSERT_EQUAL_CHAR('h', commands[2].type);
    TEST_ASSERT_EQUAL_CHAR('v', commands[3].type);
    TEST_ASSERT_EQUAL_CHAR('v', commands[4].type);
    assertFloatEqual(5.0f, commands[1].args[0]);
    assertFloatEqual(7.0f, commands[2].args[0]);
    assertFloatEqual(3.0f, commands[3].args[0]);
    assertFloatEqual(2.0f, commands[4].args[0]);

    tearDown_svg();
}
