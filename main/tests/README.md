# Unit Tests for ESP32-P4 HMI

## Test Files

- `test_json_basic.cpp` - JSON parser tests (8 tests)
- `test_svg_basic.cpp` - SVG path parser tests (14 tests)
- `test_all.hpp` - Test suite runner
- **Total: 22 test cases**

## Test Coverage

### JSON Parser Tests
1. Empty JSON parsing (should fail)
2. Minimal valid JSON
3. Invalid JSON syntax (should fail)
4. Single embedded symbol
5. Multiple symbols
6. Widget parsing
7. Wire parsing
8. Real AND gate circuit from DrawIO

### SVG Path Parser Tests
1. Move command (M)
2. Line command (L)
3. Horizontal line (H)
4. Vertical line (V)
5. Close path (Z)
6. Cubic Bezier curve (C)
7. Quadratic Bezier curve (Q)
8. Multiple line segments
9. Complex paths with mixed commands
10. Real AND gate path from DrawIO
11. OR gate path with quadratic Bezier
12. Empty string handling
13. Whitespace and comma handling
14. Relative commands

## How to Run Tests

### Option 1: Build-time Toggle (Recommended)

1. Edit `main/CMakeLists.txt`:
```cmake
idf_component_register(
    SRCS 
        "main.cpp"
        # Uncomment to enable tests:
        "tests/test_json_basic.cpp"
        "tests/test_svg_basic.cpp"
    INCLUDE_DIRS 
        "."
        "tests"
    REQUIRES 
        unity
        json_renderer
        svg_renderer
        lvgl_cpp_v9
        # ... other components
)
```

2. Edit `main/main.cpp`:
```cpp
// Set to 1 to run tests
#define RUN_TESTS 1

#if RUN_TESTS
#include "tests/test_all.hpp"
#endif

extern "C" void app_main(void)
{
#if RUN_TESTS
    // Test mode
    run_all_tests();
    
    // Keep running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
#else
    // Normal HMI mode
    init_hardware();
    // ...
#endif
}
```

3. Build and flash:
```powershell
idf.py build
idf.py -p COM3 flash monitor
```

### Option 2: Separate Test Build

Create `main_test.cpp` that only runs tests, swap with `main.cpp` during testing.

## Expected Output

```
I (123) TEST: ==============================================
I (124) TEST:   Starting Unit Tests
I (125) TEST: ==============================================

I (126) TEST: --- JSON Parser Tests ---
test_json_basic.cpp:30:test_json_parse_empty_should_fail:PASS
test_json_basic.cpp:40:test_json_parse_minimal_valid:PASS
test_json_basic.cpp:56:test_json_parse_invalid_syntax_should_fail:PASS
test_json_basic.cpp:68:test_json_parse_single_symbol:PASS
test_json_basic.cpp:100:test_json_parse_multiple_symbols:PASS
test_json_basic.cpp:136:test_json_parse_widget_basic:PASS
test_json_basic.cpp:169:test_json_parse_wires:PASS
test_json_basic.cpp:200:test_json_parse_and_gate_circuit:PASS

I (500) TEST: --- SVG Path Parser Tests ---
test_svg_basic.cpp:28:test_svg_parse_move_absolute:PASS
test_svg_basic.cpp:42:test_svg_parse_line_absolute:PASS
test_svg_basic.cpp:56:test_svg_parse_horizontal_line:PASS
test_svg_basic.cpp:72:test_svg_parse_vertical_line:PASS
test_svg_basic.cpp:88:test_svg_parse_close_path:PASS
test_svg_basic.cpp:100:test_svg_parse_cubic_bezier:PASS
test_svg_basic.cpp:120:test_svg_parse_quadratic_bezier:PASS
test_svg_basic.cpp:140:test_svg_parse_multiple_lines:PASS
test_svg_basic.cpp:154:test_svg_parse_complex_path:PASS
test_svg_basic.cpp:168:test_svg_parse_and_gate_path:PASS
test_svg_basic.cpp:200:test_svg_parse_or_gate_path:PASS
test_svg_basic.cpp:222:test_svg_parse_empty_string:PASS
test_svg_basic.cpp:234:test_svg_parse_whitespace_handling:PASS
test_svg_basic.cpp:254:test_svg_parse_relative_commands:PASS

I (800) TEST: ==============================================
I (801) TEST: ✅ ALL TESTS PASSED! (22 tests)
I (802) TEST: ==============================================

-----------------------
22 Tests 0 Failures 0 Ignored
OK
```

## Adding New Tests

1. Create test function:
```cpp
void test_my_new_feature(void) {
    setUp_json();  // or setUp_svg()
    
    // Arrange
    const char* input = "...";
    
    // Act
    bool result = myFunction(input);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    
    tearDown_json();  // or tearDown_svg()
}
```

2. Declare in `test_all.hpp`:
```cpp
extern void test_my_new_feature(void);
```

3. Add to runner:
```cpp
RUN_TEST(test_my_new_feature);
```

## Debugging Failed Tests

- Use `TEST_ASSERT_*_MESSAGE()` for better error info
- Check serial monitor for detailed failure messages
- Unity prints file:line:function:result for each test
- Failed assertions show expected vs actual values

## Performance Considerations

- Each test runs independently (setUp/tearDown)
- Total test time: ~1-2 seconds
- Memory usage: minimal (uses heap for parsers)
- Tests don't require PSRAM or display hardware

## Notes

- Tests use **mockup data**, not ported from C# yet
- Tests verify basic functionality and real DrawIO formats
- Future: Port actual C# tests for full coverage
- All tests should pass before merging code changes
