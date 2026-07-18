/*
 * Test Helper Macros and Utilities
 * 
 * Common patterns for porting C# tests to Unity framework
 */

#pragma once

#include "unity.h"
#include <string>
#include <cmath>

// ============================================================================
// String Assertion Helpers
// ============================================================================

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    TEST_ASSERT_EQUAL_STRING(expected, (actual).c_str())

#define TEST_ASSERT_STRING_CONTAINS(haystack, needle) \
    TEST_ASSERT_NOT_NULL(strstr((haystack).c_str(), needle))

#define TEST_ASSERT_STRING_NOT_EMPTY(str) \
    TEST_ASSERT_TRUE((str).length() > 0)

// ============================================================================
// Collection Assertion Helpers
// ============================================================================

#define TEST_ASSERT_COLLECTION_SIZE(expected, collection) \
    TEST_ASSERT_EQUAL_INT(expected, (collection).size())

#define TEST_ASSERT_COLLECTION_NOT_EMPTY(collection) \
    TEST_ASSERT_TRUE((collection).size() > 0)

#define TEST_ASSERT_COLLECTION_EMPTY(collection) \
    TEST_ASSERT_EQUAL_INT(0, (collection).size())

// ============================================================================
// Float/Double Comparison with Tolerances
// ============================================================================

// Standard 1% tolerance for floats
#define TEST_ASSERT_FLOAT_APPROX(expected, actual) \
    TEST_ASSERT_FLOAT_WITHIN((expected) * 0.01f, expected, actual)

// Custom tolerance
#define TEST_ASSERT_FLOAT_WITHIN_PERCENT(percent, expected, actual) \
    TEST_ASSERT_FLOAT_WITHIN((expected) * (percent) / 100.0f, expected, actual)

// Coordinate comparison (1 pixel tolerance)
#define TEST_ASSERT_COORDINATE_EQUAL(expected, actual) \
    TEST_ASSERT_FLOAT_WITHIN(1.0f, expected, actual)

// ============================================================================
// Range Assertions
// ============================================================================

#define TEST_ASSERT_IN_RANGE(value, min, max) \
    do { \
        TEST_ASSERT_TRUE((value) >= (min)); \
        TEST_ASSERT_TRUE((value) <= (max)); \
    } while(0)

#define TEST_ASSERT_POSITIVE(value) \
    TEST_ASSERT_TRUE((value) > 0)

#define TEST_ASSERT_NON_NEGATIVE(value) \
    TEST_ASSERT_TRUE((value) >= 0)

// ============================================================================
// Performance Timing Helpers
// ============================================================================

#include "esp_timer.h"

#define TEST_MEASURE_TIME_START() \
    int64_t _test_start_time = esp_timer_get_time()

#define TEST_MEASURE_TIME_END(max_us) \
    do { \
        int64_t _elapsed = esp_timer_get_time() - _test_start_time; \
        TEST_MESSAGE_FORMATTED("Elapsed: %lld us", _elapsed); \
        TEST_ASSERT_TRUE(_elapsed < (max_us)); \
    } while(0)

// ============================================================================
// Memory Testing Helpers
// ============================================================================

#include "esp_heap_caps.h"

#define TEST_ASSERT_NO_MEMORY_LEAK() \
    do { \
        size_t _heap_before = esp_get_free_heap_size(); \
        /* Test code here */ \
        size_t _heap_after = esp_get_free_heap_size(); \
        TEST_ASSERT_EQUAL_INT32(_heap_before, _heap_after); \
    } while(0)

// ============================================================================
// Exception Testing Helpers (C# → C++ port)
// ============================================================================

// C# uses Assert.Throws<Exception>()
// C++ uses try/catch or error codes

#define TEST_ASSERT_THROWS(code) \
    do { \
        bool _threw = false; \
        try { \
            code; \
        } catch (...) { \
            _threw = true; \
        } \
        TEST_ASSERT_TRUE(_threw); \
    } while(0)

#define TEST_ASSERT_NO_THROW(code) \
    do { \
        bool _threw = false; \
        try { \
            code; \
        } catch (...) { \
            _threw = true; \
        } \
        TEST_ASSERT_FALSE(_threw); \
    } while(0)

// ============================================================================
// JSON Test Helpers
// ============================================================================

#include "JsonParser.hpp"
#include "JsonTypes.hpp"

inline bool parse_json_string(const char* json, JsonRenderer::Screen& screen) {
    JsonRenderer::JsonParser parser;
    return parser.parse(json, strlen(json), screen);
}

inline bool parse_json_file(const char* filepath, JsonRenderer::Screen& screen) {
    JsonRenderer::JsonParser parser;
    return parser.parseFile(filepath, screen);
}

// ============================================================================
// SVG Test Helpers
// ============================================================================

#include "SvgPathParser.hpp"

inline std::vector<SvgRenderer::PathCommand> parse_svg_path(const char* path) {
    SvgRenderer::SvgPathParser parser;
    return parser.parse(path);
}

inline int count_svg_commands(const char* path, char cmd) {
    auto commands = parse_svg_path(path);
    int count = 0;
    for (const auto& c : commands) {
        if (c.type == cmd) count++;
    }
    return count;
}

// ============================================================================
// Test Data Helpers
// ============================================================================

// Load test file from SD card
inline std::string load_test_file(const char* filename) {
    char path[256];
    snprintf(path, sizeof(path), "/sdcard/test_data/%s", filename);
    
    FILE* f = fopen(path, "r");
    if (!f) return "";
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    std::string content(size, '\0');
    fread(&content[0], 1, size, f);
    fclose(f);
    
    return content;
}

// Compare with golden file
inline bool compare_with_golden(const uint8_t* data, size_t size, const char* golden_file) {
    std::string golden = load_test_file(golden_file);
    if (golden.size() != size) return false;
    return memcmp(data, golden.data(), size) == 0;
}

// ============================================================================
// Parametric Test Helpers (Data-Driven Tests)
// ============================================================================

#define BEGIN_PARAMETRIC_TEST(name, type) \
    struct name { type

#define END_PARAMETRIC_TEST(test_func) \
    }; \
    void test_func(const name& tc)

#define RUN_PARAMETRIC_TEST(test_func, cases) \
    do { \
        for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) { \
            test_func(cases[i]); \
        } \
    } while(0)

// Example usage:
/*
BEGIN_PARAMETRIC_TEST(AddTestCase, int)
    int a, b, expected;
END_PARAMETRIC_TEST(test_add_parametric)

void test_add_parametric(const AddTestCase& tc) {
    TEST_ASSERT_EQUAL_INT(tc.expected, tc.a + tc.b);
}

void test_addition_cases(void) {
    AddTestCase cases[] = {
        {10, 20, 30},
        {100, 200, 300},
        {-5, 5, 0}
    };
    RUN_PARAMETRIC_TEST(test_add_parametric, cases);
}
*/

// ============================================================================
// Debug Helpers
// ============================================================================

#include "esp_log.h"

#define TEST_LOG_INFO(tag, format, ...) \
    ESP_LOGI(tag, format, ##__VA_ARGS__)

#define TEST_LOG_DEBUG(tag, format, ...) \
    ESP_LOGD(tag, format, ##__VA_ARGS__)

// Conditional test (skip if condition not met)
#define TEST_SKIP_IF(condition, message) \
    if (condition) { \
        TEST_MESSAGE(message); \
        TEST_PASS(); \
        return; \
    }

// Mark test as TODO
#define TEST_TODO(message) \
    TEST_MESSAGE("TODO: " message); \
    TEST_IGNORE()

// ============================================================================
// Canvas/Rendering Test Helpers
// ============================================================================

#include "LVCanvas.hpp"

// Create test canvas
inline LVCanvas* create_test_canvas(int32_t width, int32_t height) {
    return new LVCanvas(width, height);
}

// Compare canvas pixels
inline bool compare_canvas_region(LVCanvas* canvas, int32_t x, int32_t y, 
                                 int32_t w, int32_t h, const uint8_t* expected) {
    // Implementation depends on canvas buffer format
    return true; // TODO: Implement
}

// ============================================================================
// Message Formatting Helpers
// ============================================================================

#define TEST_MESSAGE_FORMATTED(format, ...) \
    do { \
        char _msg[256]; \
        snprintf(_msg, sizeof(_msg), format, ##__VA_ARGS__); \
        TEST_MESSAGE(_msg); \
    } while(0)

// ============================================================================
// Regression Test Helpers
// ============================================================================

// Mark as regression test (bug fix verification)
#define REGRESSION_TEST(bug_id) \
    TEST_MESSAGE_FORMATTED("Regression test for bug #%d", bug_id)

// Mark as compatibility test
#define COMPATIBILITY_TEST(platform) \
    TEST_MESSAGE("Compatibility test for: " platform)
