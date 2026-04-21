# C Version - OOP Wrapper

โฟลเดอร์นี้เก็บ **C version** ของ LVGL OOP wrapper

## Files

- `lvgl_widget.c` - Base widget class (manual vtable)
- `lvgl_button.c` - Button widget
- `lvgl_label.c` - Label widget
- `lvgl_panel.c` - Panel container
- `example_oop.c` - ตัวอย่างการใช้งาน C version

## Pattern

ใช้ manual OOP pattern:
- Struct composition
- Function pointers (vtable)
- Init/cleanup functions

## Usage

ดู `example_oop.c` สำหรับตัวอย่างการใช้งาน
