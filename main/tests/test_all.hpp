/*
 * Test Suite Runner
 * 
 * Includes all test suites and provides runner function
 */

#pragma once

#include "unity.h"
#include "esp_log.h"

// ============================================================================
// Test Declarations - JSON Parser
// ============================================================================
extern void test_json_parse_empty_should_fail(void);
extern void test_json_parse_minimal_valid(void);
extern void test_json_parse_invalid_syntax_should_fail(void);
extern void test_json_parse_single_symbol(void);
extern void test_json_parse_multiple_symbols(void);
extern void test_json_parse_widget_basic(void);
extern void test_json_parse_wires(void);
extern void test_json_parse_and_gate_circuit(void);
extern void test_json_parse_snake_case_embedded_symbol_and_widget_link(void);

// ============================================================================
// Test Declarations - SVG Path Parser
// ============================================================================
extern void test_svg_parse_move_absolute(void);
extern void test_svg_parse_line_absolute(void);
extern void test_svg_parse_horizontal_line(void);
extern void test_svg_parse_vertical_line(void);
extern void test_svg_parse_close_path(void);
extern void test_svg_parse_cubic_bezier(void);
extern void test_svg_parse_quadratic_bezier(void);
extern void test_svg_parse_multiple_lines(void);
extern void test_svg_parse_complex_path(void);
extern void test_svg_parse_and_gate_path(void);
extern void test_svg_parse_or_gate_path(void);
extern void test_svg_parse_empty_string(void);
extern void test_svg_parse_whitespace_handling(void);
extern void test_svg_parse_relative_commands(void);
extern void test_svg_parse_implicit_lines_after_move(void);
extern void test_svg_parse_repeated_relative_commands_without_retyping_letter(void);

// ============================================================================
// Main Test Runner
// ============================================================================
inline void run_all_tests(void) {
    static const char* TAG = "TEST";
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "  Starting Unit Tests");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");
    
    UNITY_BEGIN();
    
    // ========================================================================
    // JSON Parser Tests (9 tests)
    // ========================================================================
    ESP_LOGI(TAG, "--- JSON Parser Tests ---");
    
    RUN_TEST(test_json_parse_empty_should_fail);
    RUN_TEST(test_json_parse_minimal_valid);
    RUN_TEST(test_json_parse_invalid_syntax_should_fail);
    RUN_TEST(test_json_parse_single_symbol);
    RUN_TEST(test_json_parse_multiple_symbols);
    RUN_TEST(test_json_parse_widget_basic);
    RUN_TEST(test_json_parse_wires);
    RUN_TEST(test_json_parse_and_gate_circuit);
    RUN_TEST(test_json_parse_snake_case_embedded_symbol_and_widget_link);
    
    // ========================================================================
    // SVG Path Parser Tests (16 tests)
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- SVG Path Parser Tests ---");
    
    RUN_TEST(test_svg_parse_move_absolute);
    RUN_TEST(test_svg_parse_line_absolute);
    RUN_TEST(test_svg_parse_horizontal_line);
    RUN_TEST(test_svg_parse_vertical_line);
    RUN_TEST(test_svg_parse_close_path);
    RUN_TEST(test_svg_parse_cubic_bezier);
    RUN_TEST(test_svg_parse_quadratic_bezier);
    RUN_TEST(test_svg_parse_multiple_lines);
    RUN_TEST(test_svg_parse_complex_path);
    RUN_TEST(test_svg_parse_and_gate_path);
    RUN_TEST(test_svg_parse_or_gate_path);
    RUN_TEST(test_svg_parse_empty_string);
    RUN_TEST(test_svg_parse_whitespace_handling);
    RUN_TEST(test_svg_parse_relative_commands);
    RUN_TEST(test_svg_parse_implicit_lines_after_move);
    RUN_TEST(test_svg_parse_repeated_relative_commands_without_retyping_letter);
    
    // ========================================================================
    // Finish
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    
    int failures = UNITY_END();
    
    if (failures == 0) {
        ESP_LOGI(TAG, "✅ ALL TESTS PASSED! (25 tests)");
    } else {
        ESP_LOGE(TAG, "❌ %d TEST(S) FAILED!", failures);
    }
    
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");
}
