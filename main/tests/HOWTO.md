# How to Enable Unit Tests

## Quick Start (3 Steps)

### Step 1: Edit main/CMakeLists.txt

Add test files to the SRCS list:

```cmake
idf_component_register(
    SRCS 
        # Main entry point
        main.cpp
        
        # === UNCOMMENT THESE LINES TO ENABLE TESTS ===
        # tests/test_json_basic.cpp
        # tests/test_svg_basic.cpp
        # === END TEST FILES ===
        
        # System Manager
        cpp_version/SystemManager.cpp
        # ... rest of sources ...
```

Also add `tests` to INCLUDE_DIRS:

```cmake
    INCLUDE_DIRS
        ../components/lvgl_cpp_v9/include
        .
        include
        examples
        tests  # <-- ADD THIS LINE
        ${LV_DEMO_DIR}
```

And add `unity` to REQUIRES:

```cmake
    REQUIRES
        unity  # <-- ADD THIS LINE (at the top)
        esp32_p4_function_ev_board
        fatfs
        svg_renderer
        json_renderer
        # ... rest of requirements ...
```

### Step 2: Edit main/main.cpp

Add test mode toggle at the top of the file:

```cpp
// ============================================================
// TEST MODE CONFIGURATION
// ============================================================
// Set to 1 to run unit tests instead of normal HMI
#define RUN_TESTS 1  // <-- Change to 1 to enable tests

#if RUN_TESTS
#include "tests/test_all.hpp"
#endif

// ============================================================
// Your normal includes...
#include "esp_log.h"
// ... rest of includes ...
```

Modify app_main():

```cpp
extern "C" void app_main(void)
{
#if RUN_TESTS
    //========================================
    // TEST MODE - Run Unit Tests
    //========================================
    ESP_LOGI("MAIN", "Running in TEST MODE");
    run_all_tests();
    
    // Keep running forever
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    
#else
    //========================================
    // NORMAL MODE - Run HMI Application
    //========================================
    ESP_LOGI("MAIN", "Running in NORMAL MODE");
    
    // Your normal initialization code...
    init_hardware();
    init_lvgl();
    // ... rest of normal code ...
#endif
}
```

### Step 3: Build and Flash

```powershell
# Build firmware
idf.py build

# Flash to ESP32-P4
idf.py -p COM3 flash monitor

# Expected output:
# I (123) TEST: ==============================================
# I (124) TEST:   Starting Unit Tests
# I (125) TEST: ==============================================
# ...
# I (800) TEST: ✅ ALL TESTS PASSED! (25 tests)
```

## Switching Back to Normal Mode

1. Set `#define RUN_TESTS 0` in main.cpp
2. Comment out test files in CMakeLists.txt
3. Rebuild: `idf.py build`

## Troubleshooting

### Compilation Errors

**Error: `unity.h` not found**
- Solution: Add `unity` to REQUIRES in CMakeLists.txt

**Error: `JsonParser.hpp` not found**
- Solution: Make sure `json_renderer` is in REQUIRES

**Error: Undefined reference to test functions**
- Solution: Uncomment test cpp files in SRCS

### Runtime Errors

**Error: Test fails immediately**
- Check serial monitor output for detailed error messages
- Verify JSON/SVG parser implementations are correct
- Use `TEST_ASSERT_*_MESSAGE()` for debugging

**Error: Crash or reset during tests**
- Increase stack size: `CONFIG_ESP_MAIN_TASK_STACK_SIZE=16384`
- Check heap usage (tests use `new`/`delete`)

## Test Structure

```
main/tests/
├── README.md                 # This file
├── test_all.hpp             # Test runner (include in main.cpp)
├── test_json_basic.cpp      # JSON parser tests (8 tests)
└── test_svg_basic.cpp       # SVG parser tests (14 tests)
```

## Example Build Output (Success)

```
[100%] Built target test_json_renderer.elf
esptool.py v4.8.1
Creating esp32p4 image...
Merged binary created successfully.

To flash all build output, run 'python C:/Users/koson/esp/v5.5.1/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32p4 -p (PORT) -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/test_json_renderer.bin'
or run 'idf.py -p (PORT) flash'

Project build complete. To flash, run:
 idf.py flash
or
idf.py -p COM3 flash
```

## Next Steps

After tests pass:
1. Review test coverage
2. Add more tests for edge cases
3. Port actual C# tests (see [TESTING_QUICKSTART.md](../TESTING_QUICKSTART.md))
4. Set up CI/CD to run tests automatically
5. Add performance benchmarks

## Tips

- Run tests frequently during development
- Add tests for every bug fix
- Keep tests fast (< 5 seconds total)
- Use descriptive test names
- Document expected behavior in test comments
