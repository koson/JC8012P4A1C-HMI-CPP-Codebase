### Documentation Files (แนะนำอ่านตามลำดับ)

1. 🎯 **[QUICKREF](QUICKREF.md)** ← **เริ่มตรงนี้!** (Quick reference, copy-paste ready)
2. 📖 **[HOWTO](HOWTO.md)**- Step-by-step setup guide
3. 📘 **[README](README.md)** - Full documentation
4. 📊 **[SUMMARY](SUMMARY.md)** - Overview + status

## 🚀 วิธีเริ่มต้น (3 นาทีเสร็จ)

### วิธีที่ 1: Toggle Mode (แนะนำ)

**1. แก้ main.cpp (บรรทัดแรกๆ):**

```c
#define RUN_TESTS 1  // 0 = HMI, 1 = Test

#if RUN_TESTS
#include "tests/test_all.hpp"
#endif
```


**2. แก้ [app_main()]:**
```c
extern "C" void app_main(void) {
#if RUN_TESTS
    run_all_tests();
    while (1) vTaskDelay(pdMS_TO_TICKS(10000));
#else
    // HMI code ตามปกติ
#endif
}
```

**3. แก้ [CMakeLists.txt]:**

```
SRCS 
    main.cpp
    tests/test_json_basic.cpp    # เพิ่มบรรทัดนี้
    tests/test_svg_basic.cpp     # เพิ่มบรรทัดนี้

INCLUDE_DIRS
    tests                        # เพิ่มบรรทัดนี้

REQUIRES
    unity                        # เพิ่มบรรทัดนี้
```

**4. Build & Flash:**

```
idf.py build

idf.py -p COM3 flash monitor
```
### วิธีที่ 2: Swap main.cpp (สำรอง)

``` powershell
cp main\main.cpp main\main.cpp.backup
cp main\main_test.cpp main\main.cpp
# แก้ CMakeLists.txt เหมือนวิธีที่ 1
idf.py build flash monitor
```

## ✅ ผลลัพธ์ที่คาดหวัง

```
I (801) TEST: ✅ ALL TESTS PASSED! (22 tests)
-----------------------
22 Tests 0 Failures 0 Ignored
OK
```

## 📊 Test Coverage

| Component   | Tests  | Coverage                                                      |
| ----------- | ------ | ------------------------------------------------------------- |
| JSON Parser | 8      | Empty, valid, invalid, symbols, widgets, wires, real circuits |
| SVG Parser  | 14     | M, L, H, V, C, Q, Z commands + DrawIO paths                   |
| **Total**   | **22** | ครอบคลุม basic + real-world data                              |

## 🎓 แนะนำให้อ่าน

1. **เริ่มต้น:** [QUICKREF](QUICKREF.md) - Copy-paste พร้อมใช้
2. **Setup:** [HOWTO](HOWTO.md)[HOWTO.md] - อธิบายละเอียด
3. **Overview:** [SUMMARY](SUMMARY.md) - สรุปภาพรวม

## ✨ จุดเด่น

✅ พร้อมใช้งานทันที (แก้แค่ 3 files)  
✅ ใช้เวลา run 1-2 วินาที  
✅ ทดสอบด้วย real DrawIO data  
✅ เปลี่ยนโหมดง่าย (#define RUN_TESTS)  
✅ Documentation ครบถ้วน

## 🎯 ขั้นตอนต่อไป

1. **ทดลอง run tests** ตาม [QUICKREF](QUICKREF.md)
2. ดูว่า 22 tests ผ่านหมดไหม
3. ถ้าผ่านหมด → ศึกษาวิธีเขียน test เพิ่ม
4. ถ้ามี error → Debug ด้วย serial monitor
5. ต่อไป port C# tests จาก WASM project (~200 tests)