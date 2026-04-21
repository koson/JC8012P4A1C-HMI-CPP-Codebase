# Examples - Widget Playground

โฟลเดอร์นี้เก็บ **example files** พร้อม template สำหรับลองเล่น

## Files

- `slider_playground.cpp` - ตัวอย่าง LVSlider ครบถ้วน (5 examples)
- `checkbox_examples.cpp` - ตัวอย่าง LVCheckbox (settings, todo list)
- `switch_examples.cpp` - ตัวอย่าง LVSwitch (control panel)
- `bar_examples.cpp` - ตัวอย่าง LVBar (progress, battery)
- `arc_examples.cpp` - ตัวอย่าง LVArc (volume, gauge)

## How to Use

### วิธีที่ 1: Auto-include (แนะนำ)
แก้ `../CMakeLists.txt` บรรทัดที่ 10:
```cmake
file(GLOB EXAMPLE_SOURCES "examples/*.cpp")  # ลบ # ออก
```

### วิธีที่ 2: เลือกไฟล์เอง
ใน `../CMakeLists.txt` เพิ่มไฟล์ที่ต้องการ:
```cmake
SRCS
    examples/slider_playground.cpp
    examples/checkbox_examples.cpp
```

## Learning Approach

1. อ่านโค้ดและ comment
2. ปรับแต่งค่าต่างๆ (marked with ❓)
3. Build และทดสอบบนบอร์ด
4. ลองสร้าง custom widgets เอง

## Template Structure

แต่ละไฟล์มี:
- ✅ Multiple examples (3-6 examples)
- ❓ TODO sections สำหรับปรับแต่ง
- 💡 Tips & tricks
- 📚 Best practices
