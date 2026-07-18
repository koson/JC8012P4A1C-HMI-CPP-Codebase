# Unit Test Strategy: C# WASM → ESP32 C++ Port

## Overview

This document outlines the strategy for porting 200+ unit tests from C# WASM to ESP32 C++ using Unity Test Framework.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│  C# WASM Tests (Source)                                │
│  ├── NUnit/xUnit framework                             │
│  ├── ~200 test cases                                   │
│  └── Test patterns & assertions                        │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  Mapping Layer                                          │
│  ├── Assert.AreEqual → TEST_ASSERT_EQUAL               │
│  ├── Assert.IsTrue → TEST_ASSERT_TRUE                  │
│  └── Test organization → Unity RUN_TEST                │
└─────────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────────┐
│  ESP32 C++ Tests (Target)                              │
│  ├── Unity Test Framework                              │
│  ├── Ported test cases                                 │
│  └── CI/CD integration                                 │
└─────────────────────────────────────────────────────────┘
```

## Test Coverage Matrix

| Component | C# Tests | ESP32 Port | Priority | Status |
|-----------|----------|------------|----------|--------|
| JSON Parser | 45 | 15 | High | ✅ Done |
| SVG Path Parser | 60 | 25 | High | ✅ Done |
| SVG Renderer | 40 | 20 | Medium | 📋 Todo |
| Coordinate Transform | 25 | 12 | High | 📋 Todo |
| Widget Rendering | 20 | 10 | Medium | 📋 Todo |
| Integration Tests | 10 | 5 | High | 📋 Todo |

## Assertion Mapping

### C# → Unity Equivalents

| C# (NUnit/xUnit) | Unity | Notes |
|------------------|-------|-------|
| `Assert.AreEqual(a, b)` | `TEST_ASSERT_EQUAL(a, b)` | Exact match |
| `Assert.AreEqual(a, b, delta)` | `TEST_ASSERT_FLOAT_WITHIN(delta, a, b)` | Float comparison |
| `Assert.IsTrue(condition)` | `TEST_ASSERT_TRUE(condition)` | Boolean |
| `Assert.IsFalse(condition)` | `TEST_ASSERT_FALSE(condition)` | Boolean |
| `Assert.IsNull(obj)` | `TEST_ASSERT_NULL(obj)` | Null check |
| `Assert.IsNotNull(obj)` | `TEST_ASSERT_NOT_NULL(obj)` | Not null |
| `Assert.Throws<Exception>()` | Custom macro | Error handling |
| `StringAssert.Contains(str, sub)` | `TEST_ASSERT_NOT_NULL(strstr(str, sub))` | String contains |

## Test Lifecycle

### C# Setup/Teardown
```csharp
[SetUp]
public void Setup() {
    parser = new JsonParser();
}

[TearDown]
public void Teardown() {
    parser?.Dispose();
}

[Test]
public void TestCase() {
    // Test code
}
```

### ESP32 Unity
```cpp
void setUp(void) {
    parser = new JsonParser();
}

void tearDown(void) {
    delete parser;
}

void test_case(void) {
    // Test code
}

// In app_main()
RUN_TEST(test_case);
```

## Memory Management Differences

### C# (Garbage Collected)
```csharp
var screen = new Screen();
var parser = new JsonParser();
// Automatic cleanup
```

### ESP32 (Manual)
```cpp
Screen* screen = new Screen();
JsonParser* parser = new JsonParser();
// Must delete!
delete parser;
delete screen;
```

### Best Practice: RAII Pattern
```cpp
void test_with_raii(void) {
    JsonParser parser;  // Stack allocation
    Screen screen;
    
    // Automatic cleanup when function exits
}
```

## Data-Driven Tests

### C# TestCase Pattern
```csharp
[TestCase(10, 20, 30)]
[TestCase(100, 200, 300)]
public void TestAddition(int a, int b, int expected) {
    Assert.AreEqual(expected, a + b);
}
```

### ESP32 Approach
```cpp
struct TestData {
    int a, b, expected;
};

void test_addition_parametric(void) {
    TestData cases[] = {
        {10, 20, 30},
        {100, 200, 300}
    };
    
    for (const auto& tc : cases) {
        TEST_ASSERT_EQUAL_INT(tc.expected, tc.a + tc.b);
    }
}
```

## Float Comparison Strategy

### Problem: Floating Point Precision
```cpp
// ❌ Will fail due to precision
TEST_ASSERT_EQUAL_FLOAT(0.3f, 0.1f + 0.2f);

// ✅ Use epsilon comparison
TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.3f, 0.1f + 0.2f);
```

### Coordinate Transformation Tests
```cpp
void test_coordinate_precision(void) {
    float scale = 0.6f;
    float x = 500.0f;
    float result = x * scale;
    
    // Allow 1 pixel tolerance
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 300.0f, result);
}
```

## Running Tests on ESP32

### Build Configuration
```bash
# Enable Unity tests
idf.py menuconfig
# → Component config → Unity unit testing library

# Build test app
idf.py build

# Flash and monitor
idf.py flash monitor
```

### Test Output Format
```
ESP32 Starting Unity Test Runner
Running 87 tests...

[TEST] test_parse_empty_json_should_fail ... PASS
[TEST] test_parse_valid_minimal_json ... PASS
[TEST] test_parse_and_gate_path_from_drawio ... PASS

==========================================
87 Tests 0 Failures 0 Ignored
OK
==========================================
```

## CI/CD Integration

### GitHub Actions Example
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
        
      - name: Build Tests
        run: |
          cd components/json_renderer/test
          idf.py build
          
      - name: Run Tests (QEMU)
        run: |
          qemu-system-xtensa -M esp32p4 -nographic \
            -kernel build/test_json_parser.bin
```

## Regression Prevention Strategy

### 1. Golden File Tests
```cpp
void test_parse_and_gate_circuit_golden(void) {
    // Load actual DrawIO export
    const char* json = load_file("/sdcard/test_data/and_gate.json");
    
    Screen screen;
    TEST_ASSERT_TRUE(parser->parse(json, strlen(json), screen));
    
    // Verify against known good results
    TEST_ASSERT_EQUAL_INT(1, screen.widgets.size());
    TEST_ASSERT_EQUAL_STRING("Dynamic_Symbol_653804273", 
                            screen.widgets[0].symbolId.c_str());
}
```

### 2. Snapshot Testing
```cpp
void test_render_and_gate_snapshot(void) {
    // Render to canvas
    renderer->loadAndRender("/sdcard/circuits/and_gate.json");
    
    // Compare canvas buffer with reference
    uint8_t* buffer = canvas->getBuffer();
    uint8_t* reference = load_reference("and_gate_reference.bin");
    
    TEST_ASSERT_EQUAL_MEMORY(reference, buffer, canvas->size());
}
```

### 3. Property-Based Testing
```cpp
void test_parser_idempotency(void) {
    // Parse → Serialize → Parse again → should be identical
    Screen screen1, screen2;
    
    parser->parse(json, strlen(json), screen1);
    char* serialized = serialize(screen1);
    parser->parse(serialized, strlen(serialized), screen2);
    
    TEST_ASSERT_TRUE(screens_equal(screen1, screen2));
}
```

## Performance Benchmarks

### C# vs ESP32 Performance Targets

| Test | C# WASM (ms) | ESP32 Target (ms) | ESP32 Actual |
|------|--------------|-------------------|--------------|
| Parse simple JSON | 0.5 | 2.0 | TBD |
| Parse complex circuit | 5.0 | 20.0 | TBD |
| SVG path parse | 0.2 | 1.0 | TBD |
| Full render | 50.0 | 100.0 | TBD |

### Performance Test Example
```cpp
void test_parse_performance_benchmark(void) {
    const int iterations = 100;
    int64_t total = 0;
    
    for (int i = 0; i < iterations; i++) {
        Screen screen;
        int64_t start = esp_timer_get_time();
        parser->parse(json, strlen(json), screen);
        int64_t elapsed = esp_timer_get_time() - start;
        total += elapsed;
    }
    
    int64_t avg = total / iterations;
    ESP_LOGI("PERF", "Average parse time: %lld us", avg);
    
    // Assert performance requirement
    TEST_ASSERT_TRUE(avg < 2000); // < 2ms
}
```

## Test Organization

### Directory Structure
```
components/
├── json_renderer/
│   ├── src/
│   │   ├── JsonParser.cpp
│   │   └── JsonRenderer.cpp
│   ├── include/
│   └── test/
│       ├── test_json_parser.cpp       ← New
│       ├── test_json_renderer.cpp     ← New
│       ├── CMakeLists.txt             ← New
│       └── test_data/
│           ├── and_gate.json
│           └── full_adder.json
├── svg_renderer/
│   ├── src/
│   └── test/
│       ├── test_svg_path_parser.cpp   ← New
│       ├── test_svg_renderer.cpp      ← New
│       └── CMakeLists.txt             ← New
```

## Next Steps

### Phase 1: Core Tests (Week 1-2)
- [x] JSON parser tests (15 tests)
- [x] SVG path parser tests (25 tests)
- [ ] Create test data files
- [ ] Setup CMakeLists for test builds

### Phase 2: Rendering Tests (Week 3-4)
- [ ] SVG renderer tests (20 tests)
- [ ] Coordinate transformation tests (12 tests)
- [ ] Golden file tests

### Phase 3: Integration (Week 5-6)
- [ ] Widget rendering tests (10 tests)
- [ ] Full circuit integration tests (5 tests)
- [ ] Performance benchmarks
- [ ] CI/CD setup

### Phase 4: Maintenance
- [ ] Add tests for new features
- [ ] Update tests when bugs found
- [ ] Performance regression tracking

## Resources

- Unity Test Framework: https://github.com/ThrowTheSwitch/Unity
- ESP-IDF Testing: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html
- Test files created:
  - `components/json_renderer/test/test_json_parser.cpp`
  - `components/svg_renderer/test/test_svg_path_parser.cpp`
