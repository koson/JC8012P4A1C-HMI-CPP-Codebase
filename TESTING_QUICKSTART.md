# Unit Testing Guide for ESP32-P4 HMI

## 📊 สถานะปัจจุบัน

### ✅ สิ่งที่เสร็จแล้ว

1. **Test Files Created:**
   - `test_json_renderer_main.cpp` - 10 tests สำหรับ JSON parsing
   - `test_svg_renderer_main.cpp` - 25 tests สำหรับ SVG path parsing
   - รวม **35 test cases** ครอบคลุม:
     * JSON parsing (empty, valid, symbols, widgets, wires)
     * SVG path parsing (M, L, H, V, C, Q, Z commands)
     * Real DrawIO circuit data

2. **Test Infrastructure:**
   - Unity test framework integration
   - Test helper macros
   - Build configuration files

### ⚠️ ปัญหาที่พบ

ESP-IDF test_apps structure มี complexity สูง:
- Dependency resolution issues
- Component path configuration ซับซ้อน
- Build time นาน (ดาวน์โหลด dependencies หลายสิบ MB)

---

## 🎯 วิธีการทดสอบ (3 แนวทาง)

### **Option 1: Integration แบบง่าย (แนะนำ!)**

ไม่ต้องสร้าง test_apps แยก แค่เพิ่ม test mode ใน main project

#### ขั้นตอน:

```powershell
# 1. สร้าง test runner ใน main
cd main
```

สร้างไฟล์ `tests/test_all.hpp`:

```cpp
#pragma once

#include "unity.h"

// Forward declarations
void test_json_parse_minimal(void);
void test_json_parse_symbols(void);
void test_svg_parse_simple_path(void);
void test_svg_parse_bezier(void);

inline void run_all_tests(void) {
    UNITY_BEGIN();
    
    // JSON tests
    RUN_TEST(test_json_parse_minimal);
    RUN_TEST(test_json_parse_symbols);
    
    // SVG tests
    RUN_TEST(test_svg_parse_simple_path);
    RUN_TEST(test_svg_parse_bezier);
    
    UNITY_END();
}
```

สร้างไฟล์ `tests/test_json_basic.cpp`:

```cpp
#include "unity.h"
#include "JsonParser.hpp"
#include "JsonTypes.hpp"

using namespace JsonRenderer;

void test_json_parse_minimal(void) {
    JsonParser parser;
    Screen screen;
    
    const char* json = R"({
        "version": "1.0",
        "title": "Test",
        "width": 1000,
        "height": 800
    })";
    
    bool result = parser.parseString(json, screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("Test", screen.title.c_str());
    TEST_ASSERT_EQUAL_INT32(1000, screen.width);
}

void test_json_parse_symbols(void) {
    JsonParser parser;
    Screen screen;
    
    const char* json = R"({
        "version": "1.0",
        "title": "Symbol Test",
        "width": 1000,
        "height": 800,
        "embeddedSymbols": {
            "gate1": {
                "id": "gate1",
                "pathData": "M 0 0 L 10 10",
                "viewBox": {"x": 0, "y": 0, "width": 100, "height": 60}
            }
        }
    })";
    
    bool result = parser.parseString(json, screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(1, screen.embeddedSymbols.size());
}
```

สร้างไฟล์ `tests/test_svg_basic.cpp`:

```cpp
#include "unity.h"
#include "SvgPathParser.hpp"

using namespace SvgRenderer;

void test_svg_parse_simple_path(void) {
    SvgPathParser parser;
    
    auto commands = parser.parse("M 10 20 L 30 40");
    
    TEST_ASSERT_EQUAL_UINT32(2, commands.size());
    TEST_ASSERT_EQUAL_CHAR('M', commands[0].type);
    TEST_ASSERT_EQUAL_CHAR('L', commands[1].type);
}

void test_svg_parse_bezier(void) {
    SvgPathParser parser;
    
    auto commands = parser.parse("M 0 0 C 10 20, 30 40, 50 60");
    
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, commands.size());
    
    // Check for C command
    bool found_c = false;
    for (const auto& cmd : commands) {
        if (cmd.type == 'C') {
            found_c = true;
            TEST_ASSERT_EQUAL_UINT32(6, cmd.args.size());
            break;
        }
    }
    TEST_ASSERT_TRUE(found_c);
}
```

แก้ไข `main/CMakeLists.txt`:

```cmake
idf_component_register(
    SRCS 
        "main.cpp"
        # Add test files (comment out when not testing)
        # "tests/test_json_basic.cpp"
        # "tests/test_svg_basic.cpp"
    INCLUDE_DIRS 
        "."
        "tests"
    REQUIRES 
        unity  # Add Unity framework
        json_renderer
        svg_renderer
        lvgl_cpp_v9
        # ... other requires ...
)
```

แก้ไข `main/main.cpp` เพิ่ม test mode:

```cpp
// At top of file
#define RUN_TESTS 0  // Set to 1 to run tests

#if RUN_TESTS
#include "unity.h"
#include "tests/test_all.hpp"
#endif

extern "C" void app_main(void)
{
#if RUN_TESTS
    // Test mode
    ESP_LOGI("TEST", "Running unit tests...");
    run_all_tests();
    
    // Keep running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
#else
    // Normal HMI mode
    init_hardware();
    init_lvgl();
    // ... normal code ...
#endif
}
```

#### Build & Run:

```powershell
# 1. แก้ #define RUN_TESTS 1
# 2. Uncomment test files ใน CMakeLists.txt
# 3. Build
idf.py build

# 4. Flash & Monitor
idf.py -p COM3 flash monitor
```

---

### **Option 2: ใช้ ESP-IDF test_apps (ซับซ้อน)**

ตาม ESP-IDF standard แต่ต้อง setup dependencies ให้ถูกต้อง

```powershell
cd components/json_renderer/test_apps
idf.py set-target esp32p4
idf.py build
idf.py -p COM3 flash monitor
```

⚠️ **ปัญหา:** ต้อง resolve component paths และ dependencies มาก

---

### **Option 3: ใช้ Host-based Testing (Native)**

Run tests บน PC โดยไม่ต้อง flash ESP32

```powershell
# Install CMocka or Unity on PC
# Build as native executable
gcc -o test_runner test_json.c json_parser.c -lunity
./test_runner
```

✅ **ข้อดี:** รวดเร็ว, debug ง่าย  
❌ **ข้อเสีย:** ไม่ได้ทดสอบบน hardware จริง

---

## 📝 Test Cases ที่พร้อมใช้

### JSON Parser Tests (10 tests)
- ✅ `test_parse_empty_string_should_fail()`
- ✅ `test_parse_valid_minimal_json()`
- ✅ `test_parse_invalid_json_should_fail()`
- ✅ `test_parse_embedded_symbol_basic()`
- ✅ `test_parse_multiple_symbols()`
- ✅ `test_parse_svg_symbol_widget()`
- ✅ `test_parse_wires()`
- ✅ `test_parse_drawio_and_gate()` - Real circuit data

### SVG Path Parser Tests (25 tests)
- ✅ `test_parse_simple_move_absolute()`
- ✅ `test_parse_simple_line_absolute()`
- ✅ `test_parse_horizontal_line()`
- ✅ `test_parse_vertical_line()`
- ✅ `test_parse_close_path()`
- ✅ `test_parse_relative_move()`
- ✅ `test_parse_relative_line()`
- ✅ `test_parse_cubic_bezier_absolute()`
- ✅ `test_parse_cubic_bezier_relative()`
- ✅ `test_parse_quadratic_bezier_absolute()`
- ✅ `test_parse_multiple_segments()`
- ✅ `test_parse_implicit_lineto()`
- ✅ `test_parse_and_gate_path_from_drawio()` - Real DrawIO path
- ✅ `test_parse_or_gate_path()`
- ✅ `test_parse_empty_string()`
- ✅ `test_parse_invalid_command()`
- ✅ `test_parse_comma_separated()`
- ✅ `test_parse_no_spaces()`
- ✅ `test_parse_extra_whitespace()`

---

## 🚀 Quick Start (แนะนำ Option 1)

```powershell
# 1. สร้าง test files
mkdir main\tests
# Copy test code จากด้านบน

# 2. แก้ main.cpp ใส่ test mode
#define RUN_TESTS 1

# 3. แก้ CMakeLists.txt เพิ่ม test files

# 4. Build & Flash
idf.py build
idf.py -p COM3 flash monitor

# 5. ดูผลลัพธ์
# Serial monitor จะแสดง:
# - Test results
# - Pass/Fail count
# - Error messages (if any)
```

---

## 📚 เพิ่มเติม

### Adding New Tests

```cpp
// In tests/test_myfeature.cpp
#include "unity.h"

void test_my_new_feature(void) {
    // Arrange
    int expected = 42;
    
    // Act
    int actual = my_function();
    
    // Assert
    TEST_ASSERT_EQUAL_INT32(expected, actual);
}
```

### Debugging Failed Tests

```cpp
// Use TEST_ASSERT_MESSAGE for better error messages
TEST_ASSERT_TRUE_MESSAGE(result, "JSON parsing failed");
TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, "Title mismatch");
```

### Performance Testing

```cpp
#include "esp_timer.h"

void test_json_parse_performance(void) {
    int64_t start = esp_timer_get_time();
    
    // Run test
    parser.parseString(largeJson, screen);
    
    int64_t elapsed = esp_timer_get_time() - start;
    
    TEST_ASSERT_LESS_THAN_INT64(100000, elapsed); // < 100ms
}
```

---

## 🎯 สรุป

**แนะนำ Option 1** เพราะ:
- ✅ ง่ายที่สุด
- ✅ Build เร็ว
- ✅ Debug ง่าย
- ✅ ไม่ต้อง setup ซับซ้อน

**ขั้นตอนถัดไป:**
1. สร้าง `main/tests/` directory
2. Copy test code ที่เตรียมไว้
3. แก้ `main.cpp` เพิ่ม test mode
4. Build & test บน ESP32

**สำหรับ production:**
- ปิด test mode (#define RUN_TESTS 0)
- Comment out test files ใน CMakeLists.txt
- Build normal firmware

---

ต้องการให้ผมช่วยสร้าง test files เหล่านี้ไหมครับ?
