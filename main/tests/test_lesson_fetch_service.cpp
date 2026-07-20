/*
 * LessonFetchService tests (test-first guard rails)
 */

#include "unity.h"
#include "LessonFetchService.hpp"
#include <string.h>

void test_fetch_url_accepts_http(void)
{
    TEST_ASSERT_TRUE(LessonFetch::isSupportedUrl("http://example.com/lesson.json"));
}

void test_fetch_url_accepts_https(void)
{
    TEST_ASSERT_TRUE(LessonFetch::isSupportedUrl("https://example.com/lesson.json"));
}

void test_fetch_url_rejects_non_http_scheme(void)
{
    TEST_ASSERT_FALSE(LessonFetch::isSupportedUrl("ftp://example.com/lesson.json"));
}

void test_fetch_auth_accepts_pair_or_empty(void)
{
    TEST_ASSERT_TRUE(LessonFetch::isValidAuthInput(nullptr, nullptr));
    TEST_ASSERT_TRUE(LessonFetch::isValidAuthInput("student", "LabBuddy@2026"));
}

void test_fetch_auth_rejects_half_pair(void)
{
    TEST_ASSERT_FALSE(LessonFetch::isValidAuthInput("student", nullptr));
    TEST_ASSERT_FALSE(LessonFetch::isValidAuthInput(nullptr, "LabBuddy@2026"));
}

void test_fetch_sanitize_dir_accepts_valid_token(void)
{
    char out[32] = {0};
    TEST_ASSERT_TRUE(LessonFetch::sanitizeDirName("lessons", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("lessons", out);
}

void test_fetch_sanitize_dir_rejects_path_traversal_chars(void)
{
    char out[32] = {0};
    TEST_ASSERT_FALSE(LessonFetch::sanitizeDirName("../secret", out, sizeof(out)));
}

void test_fetch_derive_filename_from_url_strips_query(void)
{
    char out[64] = {0};
    TEST_ASSERT_TRUE(LessonFetch::deriveFilenameFromUrl("https://host/path/L001_NOT_GATE.json?token=abc", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("L001_NOT_GATE.json", out);
}

void test_fetch_ensure_json_extension_appends_when_missing(void)
{
    char name[64] = "lesson_001";
    TEST_ASSERT_TRUE(LessonFetch::ensureJsonExtension(name, sizeof(name)));
    TEST_ASSERT_EQUAL_STRING("lesson_001.JSON", name);
}

void test_fetch_build_sd_path_formats_correctly(void)
{
    char path[128] = {0};
    TEST_ASSERT_TRUE(LessonFetch::buildSdFilePath("lessons", "L001.JSON", path, sizeof(path)));
    TEST_ASSERT_EQUAL_STRING("/sdcard/lessons/L001.JSON", path);
}
