# C++ Version - OOP Wrapper

โฟลเดอร์นี้เก็บ **C++ version** ของ LVGL OOP wrapper

## Files

### Core Widgets
- `LVWidget.cpp` - Base widget class
- `LVButton.cpp` - Button widget
- `LVLabel.cpp` - Label widget
- `LVPanel.cpp` - Panel container

### Input Controls (Batch 1)
- `LVSlider.cpp` - Slider (แถบเลื่อน)
- `LVCheckbox.cpp` - Checkbox (ช่องติ๊ก)
- `LVSwitch.cpp` - Switch (สวิตช์)
- `LVBar.cpp` - Progress bar
- `LVArc.cpp` - Arc (วงโค้ง)

### Examples
- `example_cpp.cpp` - CounterApp demo

## Pattern

ใช้ Modern C++ features:
- Virtual functions
- std::function callbacks
- Lambda expressions
- RAII
- Method chaining

## Usage

ดู `example_cpp.cpp` และ `../examples/*.cpp` สำหรับตัวอย่าง

## Learning

อ่าน `../../Docs/PATTERN_GUIDE.md` เพื่อเรียนรู้วิธีสร้าง widget ใหม่
