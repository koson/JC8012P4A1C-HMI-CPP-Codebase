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
// Test Declarations - JsonRenderer Contract
// ============================================================================
extern void test_renderer_path_widget_generates_fill_and_stroke_commands(void);
extern void test_renderer_label_widget_generates_text_command(void);
extern void test_renderer_rect_widget_generates_rect_and_border_lines(void);
extern void test_renderer_circle_widget_uses_ellipse_and_clamps_stroke_width(void);
extern void test_renderer_draw_order_wires_then_widgets_then_ports(void);
extern void test_renderer_missing_symbol_is_skipped(void);
extern void test_renderer_wire_stroke_width_is_clamped(void);
extern void test_renderer_port_radius_is_capped(void);

// ============================================================================
// Test Declarations - LessonFetchService
// ============================================================================
extern void test_fetch_url_accepts_http(void);
extern void test_fetch_url_accepts_https(void);
extern void test_fetch_url_rejects_non_http_scheme(void);
extern void test_fetch_auth_accepts_pair_or_empty(void);
extern void test_fetch_auth_rejects_half_pair(void);
extern void test_fetch_sanitize_dir_accepts_valid_token(void);
extern void test_fetch_sanitize_dir_rejects_path_traversal_chars(void);
extern void test_fetch_derive_filename_from_url_strips_query(void);
extern void test_fetch_ensure_json_extension_appends_when_missing(void);
extern void test_fetch_build_sd_path_formats_correctly(void);

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
    // JsonRenderer Contract Tests (8 tests)
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- JsonRenderer Contract Tests ---");

    RUN_TEST(test_renderer_path_widget_generates_fill_and_stroke_commands);
    RUN_TEST(test_renderer_label_widget_generates_text_command);
    RUN_TEST(test_renderer_rect_widget_generates_rect_and_border_lines);
    RUN_TEST(test_renderer_circle_widget_uses_ellipse_and_clamps_stroke_width);
    RUN_TEST(test_renderer_draw_order_wires_then_widgets_then_ports);
    RUN_TEST(test_renderer_missing_symbol_is_skipped);
    RUN_TEST(test_renderer_wire_stroke_width_is_clamped);
    RUN_TEST(test_renderer_port_radius_is_capped);

    // ========================================================================
    // LessonFetchService Tests (10 tests)
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "--- LessonFetchService Tests ---");

    RUN_TEST(test_fetch_url_accepts_http);
    RUN_TEST(test_fetch_url_accepts_https);
    RUN_TEST(test_fetch_url_rejects_non_http_scheme);
    RUN_TEST(test_fetch_auth_accepts_pair_or_empty);
    RUN_TEST(test_fetch_auth_rejects_half_pair);
    RUN_TEST(test_fetch_sanitize_dir_accepts_valid_token);
    RUN_TEST(test_fetch_sanitize_dir_rejects_path_traversal_chars);
    RUN_TEST(test_fetch_derive_filename_from_url_strips_query);
    RUN_TEST(test_fetch_ensure_json_extension_appends_when_missing);
    RUN_TEST(test_fetch_build_sd_path_formats_correctly);
    
    // ========================================================================
    // Finish
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    
    int failures = UNITY_END();
    
    if (failures == 0) {
        ESP_LOGI(TAG, "✅ ALL TESTS PASSED! (43 tests)");
    } else {
        ESP_LOGE(TAG, "❌ %d TEST(S) FAILED!", failures);
    }
    
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");
}
