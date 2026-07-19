# Unit Testing Quick Reference

## 🚀 Quick Start (Copy-Paste Ready)

### Method 1: Toggle in main.cpp (Recommended)

**1. Add to top of main/main.cpp:**
```cpp
#define RUN_TESTS 1  // Set to 0 for normal mode

#if RUN_TESTS
#include "tests/test_all.hpp"
#endif
```

**2. Wrap app_main:**
```cpp
extern "C" void app_main(void)
{
#if RUN_TESTS
    run_all_tests();
    while (1) vTaskDelay(pdMS_TO_TICKS(10000));
#else
    // Normal HMI code here
#endif
}
```

**3. Edit main/CMakeLists.txt:**
```cmake
SRCS 
    main.cpp
    tests/test_json_basic.cpp    # ADD THIS
    tests/test_svg_basic.cpp     # ADD THIS
    # ... rest of sources

INCLUDE_DIRS
    .
    tests                        # ADD THIS
    # ... rest of includes

REQUIRES
    unity                        # ADD THIS
    json_renderer
    svg_renderer
    # ... rest of requires
```

**4. Build:**
```powershell
idf.py build
idf.py -p COM3 flash monitor
```

---

### Method 2: Swap main.cpp (Alternative)

```powershell
# Switch to test mode
cp main\main.cpp main\main.cpp.backup
cp main\main_test.cpp main\main.cpp

# Build and test
idf.py build
idf.py -p COM3 flash monitor

# Switch back to normal mode
cp main\main.cpp.backup main\main.cpp
idf.py build
```

---

## 📊 Test Overview

| Test Suite | Tests | Coverage |
|------------|-------|----------|
| JSON Parser | 8 | Parsing, symbols, widgets, wires, real circuits |
| SVG Parser | 14 | M, L, H, V, C, Q, Z commands, DrawIO paths |
| **Total** | **22** | Basic + Real-world data |

---

## ✅ Expected Output

```
I (125) TEST: ==============================================
I (126) TEST:   Starting Unit Tests
I (127) TEST: ==============================================

I (128) TEST: --- JSON Parser Tests ---
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
I (801) TEST: ✅ ALL TESTS PASSED! (25 tests)
I (802) TEST: ==============================================

-----------------------
25 Tests 0 Failures 0 Ignored
OK
```

---

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| `unity.h not found` | Add `unity` to REQUIRES in CMakeLists.txt |
| `undefined reference` | Uncomment test .cpp files in SRCS |
| Test crashes | Increase `CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384` |
| Heap overflow | Tests allocate parsers - check available heap |

---

## 📝 Adding New Tests

```cpp
// In test_json_basic.cpp or test_svg_basic.cpp

void test_my_new_feature(void) {
    setUp_json();  // or setUp_svg()
    
    // Test code here
    TEST_ASSERT_TRUE(condition);
    
    tearDown_json();  // or tearDown_svg()
}
```

**Then in test_all.hpp:**
```cpp
extern void test_my_new_feature(void);

// In run_all_tests():
RUN_TEST(test_my_new_feature);
```

---

## 📁 File Structure

```
main/tests/
├── HOWTO.md              # Detailed instructions
├── QUICKREF.md           # This file
├── README.md             # Test documentation
├── test_all.hpp          # Test runner
├── test_json_basic.cpp   # JSON tests (8 tests)
└── test_svg_basic.cpp    # SVG tests (14 tests)
```

---

## 🔄 Workflow

```
┌─────────────────┐
│  Write Code     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Enable Tests   │  #define RUN_TESTS 1
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Build & Flash  │  idf.py build flash monitor
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Check Results  │  25 Tests 0 Failures?
└────────┬────────┘
         │
    ┌────┴────┐
    │ PASS?   │
    └────┬────┘
    Yes  │  No
    ▼    │    ▼
┌───────┐│  ┌──────────┐
│ Ship! ││  │ Fix Bugs │
└───────┘│  └────┬─────┘
         │       │
         │       └───────┐
         │               │
         ▼               ▼
┌─────────────────┐  ┌────────────┐
│  Disable Tests  │  │  Re-test   │
│  #define 0      │  └──────┬─────┘
└─────────────────┘         │
                            └──────────┘
```

---

## ⚡ Performance

- **Build time:** +5-10 seconds (when tests enabled)
- **Flash size:** +~50KB
- **Test duration:** 1-2 seconds
- **Memory usage:** Minimal (~10KB heap)

---

## 🎯 Tips

✅ **DO:**
- Run tests before committing code
- Add test for every bug fix
- Use descriptive test names
- Keep tests independent

❌ **DON'T:**
- Leave tests enabled in production
- Share state between tests
- Test too many things in one test
- Forget to call setUp/tearDown

---

## 📚 Resources

- [HOWTO.md](HOWTO.md) - Step-by-step setup
- [README.md](README.md) - Full documentation
- [TESTING_QUICKSTART.md](../TESTING_QUICKSTART.md) - Testing strategies
- [Unity Framework](https://github.com/ThrowTheSwitch/Unity) - Test framework docs

---

**Last Updated:** 2026-07-19  
**Test Count:** 25 tests
**Status:** ✅ Ready to use
