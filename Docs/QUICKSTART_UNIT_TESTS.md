# Quick Start: Porting C# Unit Tests to ESP32

## 🎯 Goal
Port your 200+ C# WASM unit tests to ESP32 C++ for regression prevention.

## 📦 What You Get

After following this guide, you'll have:
- ✅ Unit tests running on real ESP32 hardware
- ✅ Automated regression detection
- ✅ CI/CD integration ready
- ✅ Same test coverage as C# WASM

---

## 🚀 Quick Start (5 minutes)

### 1. Run Example Tests

```powershell
# Navigate to project root
cd d:\GitHubRepos\LabBuddy\Firmware\LabBuddy-JC8012P4-HMI

# Run JSON parser tests
.\run_tests.ps1 json_parser -Monitor

# Run SVG parser tests
.\run_tests.ps1 svg_parser -Monitor

# Run all tests
.\run_tests.ps1 all -Monitor
```

### 2. Expected Output

```
========================================
ESP32 Unit Test Runner
========================================

Testing: json_parser
----------------------------------------
Building json_parser tests...
✓ Build successful

Flashing json_parser tests...
Using port: COM3
✓ Flash successful

Starting monitor...

ESP32 Starting Unity Test Runner
Running 15 tests...

[TEST] test_parse_empty_json_should_fail ... PASS
[TEST] test_parse_valid_minimal_json ... PASS
[TEST] test_parse_embedded_symbol_with_path_data ... PASS
...

==========================================
15 Tests 0 Failures 0 Ignored
OK
==========================================

✓ json_parser: PASSED

Test Summary
========================================
Total: 2 | Passed: 2 | Failed: 0

All tests passed!
```

---

## 📝 How to Port Your C# Tests

### Step 1: Identify Test Category

Your C# test categories map to ESP32 components:

| C# Test File | ESP32 Component | Priority |
|--------------|-----------------|----------|
| `JsonParserTests.cs` | `json_renderer/test` | High |
| `SvgPathParserTests.cs` | `svg_renderer/test` | High |
| `CoordinateTests.cs` | `json_renderer/test` | High |
| `RenderingTests.cs` | `json_renderer/test` | Medium |

### Step 2: Convert Assertion Syntax

```csharp
// C# (NUnit)
[Test]
public void ParseValidJson_ShouldSucceed() {
    var parser = new JsonParser();
    var screen = new Screen();
    
    bool result = parser.Parse(json, screen);
    
    Assert.IsTrue(result);
    Assert.AreEqual("Test", screen.Title);
    Assert.AreEqual(1000, screen.Width);
}
```

```cpp
// ESP32 (Unity)
void test_parse_valid_json_should_succeed(void) {
    JsonParser parser;
    Screen screen;
    
    bool result = parser.parse(json, strlen(json), screen);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("Test", screen.title.c_str());
    TEST_ASSERT_EQUAL_INT(1000, screen.width);
}
```

### Step 3: Use Test Helpers

```cpp
#include "test_helpers.hpp"

void test_with_helpers(void) {
    // Instead of manual assertions
    TEST_ASSERT_STRING_EQUAL("expected", myString);
    TEST_ASSERT_COLLECTION_SIZE(3, myVector);
    TEST_ASSERT_FLOAT_APPROX(0.3f, result);
    
    // Performance testing
    TEST_MEASURE_TIME_START();
    // ... test code ...
    TEST_MEASURE_TIME_END(5000); // < 5ms
}
```

### Step 4: Add to Test Runner

```cpp
// In test_json_parser.cpp

extern "C" void app_main(void) {
    UNITY_BEGIN();
    
    // Your new test
    RUN_TEST(test_my_new_feature);
    
    UNITY_END();
}
```

---

## 🎨 Common Porting Patterns

### Pattern 1: Data-Driven Tests

```csharp
// C# TestCase
[TestCase(10, 20, 30)]
[TestCase(100, 200, 300)]
public void TestAdd(int a, int b, int expected) {
    Assert.AreEqual(expected, a + b);
}
```

```cpp
// ESP32 Equivalent
void test_add_parametric(void) {
    struct TestCase { int a, b, expected; };
    TestCase cases[] = {
        {10, 20, 30},
        {100, 200, 300}
    };
    
    for (const auto& tc : cases) {
        TEST_ASSERT_EQUAL_INT(tc.expected, tc.a + tc.b);
    }
}
```

### Pattern 2: Setup/Teardown

```csharp
// C#
[SetUp]
public void Setup() {
    parser = new JsonParser();
}

[TearDown]
public void Teardown() {
    parser?.Dispose();
}
```

```cpp
// ESP32
void setUp(void) {
    parser = new JsonParser();
}

void tearDown(void) {
    delete parser;
    parser = nullptr;
}
```

### Pattern 3: Exception Testing

```csharp
// C#
[Test]
public void InvalidJson_ShouldThrow() {
    Assert.Throws<JsonException>(() => {
        parser.Parse(invalidJson);
    });
}
```

```cpp
// ESP32 (error code style)
void test_invalid_json_should_fail(void) {
    bool result = parser->parse(invalidJson, strlen(invalidJson), screen);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NOT_NULL(parser->getLastError());
}
```

### Pattern 4: Float Comparison

```csharp
// C#
Assert.AreEqual(0.3, 0.1 + 0.2, 0.0001);
```

```cpp
// ESP32
TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.3f, 0.1f + 0.2f);
// Or use helper
TEST_ASSERT_FLOAT_APPROX(0.3f, 0.1f + 0.2f); // 1% tolerance
```

---

## 🗂️ Test File Organization

```
components/
├── json_renderer/
│   └── test/
│       ├── test_json_parser.cpp         ← Port your JsonParserTests.cs here
│       ├── test_json_renderer.cpp       ← Port your RenderingTests.cs here
│       ├── test_coordinate_transform.cpp ← Port your CoordinateTests.cs here
│       ├── test_helpers.hpp             ← Shared helpers
│       ├── CMakeLists.txt               ← Build config
│       └── test_data/                   ← Test JSON files
│           ├── and_gate_simple.json
│           └── full_adder.json
│
└── svg_renderer/
    └── test/
        ├── test_svg_path_parser.cpp     ← Port your SvgPathTests.cs here
        ├── test_svg_renderer.cpp        ← Port your SvgRenderTests.cs here
        └── CMakeLists.txt
```

---

## 🔄 Workflow

### Daily Development

```powershell
# 1. Make code changes
# 2. Run affected tests
.\run_tests.ps1 json_parser

# 3. If tests fail, fix code
# 4. Repeat until tests pass
```

### Adding New Feature

```powershell
# 1. Port C# test first (TDD)
# Edit: components/json_renderer/test/test_json_parser.cpp

# 2. Run test (should fail)
.\run_tests.ps1 json_parser

# 3. Implement feature
# Edit: components/json_renderer/src/JsonParser.cpp

# 4. Run test (should pass)
.\run_tests.ps1 json_parser
```

### Before Commit

```powershell
# Run ALL tests to ensure no regressions
.\run_tests.ps1 all

# Commit only if all tests pass
git commit -m "Add new feature with tests"
```

---

## 📊 Test Status Tracker

Track your porting progress:

| C# Test Suite | Tests | Ported | Status | Priority |
|---------------|-------|--------|--------|----------|
| JsonParserTests.cs | 45 | 15 | 🟡 In Progress | High |
| SvgPathParserTests.cs | 60 | 25 | 🟡 In Progress | High |
| CoordinateTests.cs | 25 | 0 | ⚪ Not Started | High |
| RenderingTests.cs | 40 | 0 | ⚪ Not Started | Medium |
| IntegrationTests.cs | 30 | 0 | ⚪ Not Started | Medium |

---

## 🐛 Debugging Failed Tests

### Test Fails on ESP32 but Passes in C#

**Possible Causes:**
1. **Float precision**: ESP32 FPU may have slight differences
   ```cpp
   // Use tolerance
   TEST_ASSERT_FLOAT_WITHIN(0.01f, expected, actual);
   ```

2. **Memory alignment**: ESP32 requires aligned access
   ```cpp
   // Use memcpy for unaligned data
   float value;
   memcpy(&value, unaligned_ptr, sizeof(float));
   ```

3. **Endianness**: ESP32 is little-endian (usually not an issue)

4. **Stack size**: ESP32 has limited stack
   ```cpp
   // Move large buffers to heap
   std::vector<uint8_t> buffer(large_size); // Uses heap
   ```

### Test Crashes ESP32

**Debug Steps:**
1. Check serial output for panic trace
2. Use `TEST_SKIP_IF` to isolate failing test
3. Reduce test complexity
4. Check memory usage with `esp_get_free_heap_size()`

```cpp
void test_debug_crash(void) {
    size_t free_before = esp_get_free_heap_size();
    ESP_LOGI("TEST", "Free heap before: %d", free_before);
    
    // Test code here
    
    size_t free_after = esp_get_free_heap_size();
    ESP_LOGI("TEST", "Free heap after: %d", free_after);
}
```

---

## 🚀 Advanced: CI/CD Integration

### GitHub Actions Example

Create `.github/workflows/esp32-tests.yml`:

```yaml
name: ESP32 Unit Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup ESP-IDF
        uses: espressif/esp-idf-ci-action@v1
        with:
          esp_idf_version: v5.4
          target: esp32p4
          
      - name: Build JSON Parser Tests
        run: |
          cd components/json_renderer/test
          idf.py build
          
      - name: Build SVG Parser Tests
        run: |
          cd components/svg_renderer/test
          idf.py build
          
      # Optional: Run tests in QEMU
      - name: Run Tests (QEMU)
        run: |
          # QEMU support for ESP32-P4 coming soon
          echo "Tests built successfully"
```

---

## 📚 Resources

### Documentation
- [UNIT_TEST_PORTING_STRATEGY.md](UNIT_TEST_PORTING_STRATEGY.md) - Full strategy
- [ESP-IDF Unit Testing](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)
- [Unity Framework](https://github.com/ThrowTheSwitch/Unity)

### Example Tests
- `components/json_renderer/test/test_json_parser.cpp` - 15 JSON parsing tests
- `components/svg_renderer/test/test_svg_path_parser.cpp` - 25 SVG path tests

### Scripts
- `run_tests.ps1` - Test runner script
- `components/*/test/CMakeLists.txt` - Build configuration

---

## 💡 Tips for Success

### ✅ DO:
- Port tests incrementally (10-20 at a time)
- Use test helpers for common patterns
- Run tests frequently during development
- Add regression tests for bugs
- Use descriptive test names
- Group related tests together

### ❌ DON'T:
- Port all 200 tests at once
- Ignore failed tests
- Skip test data validation
- Forget to clean up resources (delete/free)
- Use exact float equality
- Allocate large buffers on stack

---

## 🎯 Next Steps

1. **Port your first 10 tests** (30 minutes)
   - Choose highest priority test file
   - Use examples as template
   - Run and verify

2. **Add test data files** (15 minutes)
   - Copy JSON files to `test_data/`
   - Verify file loading

3. **Integrate into workflow** (15 minutes)
   - Add to pre-commit hook
   - Run before each build

4. **Measure coverage** (ongoing)
   - Track porting progress
   - Identify gaps

---

**Ready to start?** Run your first test now:

```powershell
.\run_tests.ps1 json_parser -Monitor
```

Good luck! 🚀
