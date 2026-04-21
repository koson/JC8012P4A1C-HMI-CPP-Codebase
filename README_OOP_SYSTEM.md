# LVGL OOP Wrapper - System Overview

## 📋 Project Summary

โปรเจคนี้สร้างระบบ **Object-Oriented Programming (OOP) Wrapper** สำหรับ LVGL v9 โดยใช้หลักการเดียวกับที่ **Microsoft Foundation Classes (MFC)** ห่อหุ้ม WIN32 API

### 🎯 Objectives

- ✅ สร้าง Base Class สำหรับ Widgets (LVWidget)
- ✅ สร้าง Derived Classes (LVButton, LVLabel, LVPanel)
- ✅ รองรับ Inheritance, Polymorphism, Encapsulation
- ✅ ระบบ Event Handling แบบ MFC-style
- ✅ Type-safe API
- ✅ Memory management แบบ RAII-like

## 📁 File Structure

```
LVGL_v9_learn/main/
│
├── include/                    # Header files
│   ├── lvgl_widget.h          # Base widget class
│   ├── lvgl_button.h          # Button widget class
│   ├── lvgl_label.h           # Label widget class
│   └── lvgl_panel.h           # Panel/Container widget class
│
├── lvgl_widget.c              # Base widget implementation
├── lvgl_button.c              # Button implementation
├── lvgl_label.c               # Label implementation
├── lvgl_panel.c               # Panel implementation
├── example_oop.c              # Complete OOP examples
│
├── main.c                     # Application entry point
├── widgets.c                  # Legacy widget examples
└── CMakeLists.txt             # Build configuration
│
├── OOP_WRAPPER_README.md      # User documentation
├── OOP_TECHNICAL_DESIGN.md    # Technical design doc
└── QUICK_START.md             # Quick start guide
```

## 🏗️ Architecture

### Class Hierarchy

```
┌─────────────────────────────┐
│      LVWidget (Base)        │
│  ┌──────────────────────┐   │
│  │ - vtable             │   │
│  │ - obj (lv_obj_t*)   │   │
│  │ - properties         │   │
│  │ - event_handlers     │   │
│  └──────────────────────┘   │
└──────────┬──────────────────┘
           │
           ├── LVButton
           │   ├── Inherits: LVWidget
           │   ├── Adds: label, text, font
           │   └── Methods: set_text(), set_font()
           │
           ├── LVLabel
           │   ├── Inherits: LVWidget
           │   ├── Adds: text, font, color
           │   └── Methods: set_text(), set_text_fmt()
           │
           └── LVPanel
               ├── Inherits: LVWidget
               ├── Adds: layout, padding, gap
               └── Methods: set_flex_flow(), set_padding()
```

### OOP Mechanisms in C

| OOP Feature | C Implementation |
|-------------|------------------|
| **Inheritance** | Struct composition (base as first member) |
| **Polymorphism** | Virtual function table (vtable) |
| **Encapsulation** | Private data in struct, public API functions |
| **Constructor** | `lv{type}_create()` function |
| **Destructor** | `lv{type}_destroy()` or vtable->destroy |

## 🔑 Key Features

### 1. Type Safety

```c
// MFC-style type safety
LVButton* btn = lvbutton_create(NULL);
LVWidget* widget = LVBUTTON_TO_WIDGET(btn);  // Safe upcast

// Type-specific methods
lvbutton_set_text(btn, "Click Me");

// Common methods (via base class)
lvwidget_set_size(widget, 100, 50);
```

### 2. Virtual Functions

```c
// Base class defines interface
struct LVWidgetVTable {
    void (*destroy)(LVWidget* widget);
    void (*on_clicked)(LVWidget* widget, lv_event_t* event);
};

// Derived class overrides
static const LVWidgetVTable button_vtable = {
    .destroy = button_destroy,      // Override
    .on_clicked = button_on_clicked // Override
};

// Polymorphic call
widget->vtable->destroy(widget);  // Calls appropriate version
```

### 3. Event System

```c
// MFC-style event handling (like ON_BN_CLICKED)
void on_button_clicked(LVWidget* widget, lv_event_t* event) {
    ESP_LOGI("UI", "Button clicked!");
}

lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn), on_button_clicked);
```

### 4. Properties Management

```c
// Cached properties
struct {
    int32_t x, y, width, height;
    bool visible, enabled;
} properties;

// Synchronized with LVGL
lvwidget_set_size(widget, 100, 50);  // Updates both cache and LVGL
```

## 📊 Comparison Table

### MFC vs LVGL OOP Wrapper

| MFC Class | LVGL Wrapper | Description |
|-----------|--------------|-------------|
| `CWnd` | `LVWidget` | Base window class |
| `CButton` | `LVButton` | Button control |
| `CStatic` | `LVLabel` | Static text/label |
| `CDialog` | `LVPanel` | Dialog/Panel container |
| `SetWindowText()` | `lvbutton_set_text()` | Set text |
| `GetWindowText()` | `lvbutton_get_text()` | Get text |
| `ShowWindow()` | `lvwidget_show()` | Show widget |
| `EnableWindow()` | `lvwidget_enable()` | Enable widget |
| `ON_BN_CLICKED` | `lvwidget_set_on_clicked()` | Click event |
| `UpdateData(FALSE)` | `dialog_update_data()` | Update UI |
| `OnInitDialog()` | `dialog_init()` | Initialize |
| `virtual ~CWnd()` | `vtable->destroy()` | Destructor |

## 🎨 Usage Examples

### Example 1: Simple Button

```c
LVButton* btn = lvbutton_create(NULL);
lvbutton_set_text(btn, "Click Me");
lvwidget_set_size(LVBUTTON_TO_WIDGET(btn), 150, 60);
lvwidget_set_align(LVBUTTON_TO_WIDGET(btn), LV_ALIGN_CENTER, 0, 0);
lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn), on_btn_clicked);
```

### Example 2: Counter Application

```c
// Global state
static int counter = 0;
static LVLabel* lbl_counter;

// Event handler
void on_increment(LVWidget* widget, lv_event_t* event) {
    counter++;
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
}

// Create UI
void create_ui(void) {
    LVPanel* panel = lvpanel_create(NULL);
    lvpanel_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    
    lbl_counter = lvlabel_create(LVPANEL_TO_WIDGET(panel));
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
    
    LVButton* btn = lvbutton_create(LVPANEL_TO_WIDGET(panel));
    lvbutton_set_text(btn, "+");
    lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn), on_increment);
}
```

### Example 3: MFC Dialog Style

```c
typedef struct {
    // Member variables
    LVPanel* panel;
    LVButton* ok_btn;
    LVLabel* label;
    
    // Data
    int value;
} MyDialog;

void mydialog_init(MyDialog* dlg) {
    dlg->value = 0;
    dlg->panel = lvpanel_create(NULL);
    // ... create controls
}

void mydialog_update_data(MyDialog* dlg, bool to_controls) {
    if (to_controls) {
        lvlabel_set_text_fmt(dlg->label, "Value: %d", dlg->value);
    }
}
```

## 🔧 Build & Run

### Build Project

```bash
# ESP-IDF command
idf.py build

# Or use VS Code ESP-IDF extension
# Ctrl+Shift+P -> ESP-IDF: Build Project
```

### Flash to Device

```bash
idf.py -p COM3 flash monitor

# Or use VS Code ESP-IDF extension
```

### Expected Output

```
I (1234) MAIN: Starting Multi-Tab Display System...
I (1235) LVWidget: Widget initialized: 0x3ffc0000
I (1236) LVPanel: Panel created: 0x3ffc0000
I (1237) LVButton: Button created: 0x3ffc0100
I (1238) LVLabel: Label created: 0x3ffc0200
I (1239) MAIN: System ready! OOP UI is running.
```

## 📈 Performance & Memory

### Memory Usage

| Widget Type | Size (bytes) | Overhead vs Raw LVGL |
|-------------|--------------|---------------------|
| LVWidget | ~100 | +60 bytes |
| LVButton | ~120 | +80 bytes |
| LVLabel | ~120 | +80 bytes |
| LVPanel | ~110 | +70 bytes |

**Note:** Overhead มาจาก vtable pointer, properties cache, และ event handlers

### Performance

- Function call overhead: 2-3 extra calls per operation
- Negligible impact สำหรับ UI operations
- Trade-off: Type safety & Convenience vs Performance

## 🎓 Design Patterns Used

1. **Virtual Function Pattern** - Polymorphism via function pointers
2. **Template Method Pattern** - Base class defines skeleton, derived overrides
3. **Strategy Pattern** - Event handlers as interchangeable strategies
4. **Factory Pattern** - `create()` functions as factories
5. **Composite Pattern** - Parent-child widget hierarchy
6. **Observer Pattern** - Event system

## 📚 Learning Resources

### Understanding the Code

1. **Start here:** `QUICK_START.md` - ตัวอย่างการใช้งาน
2. **Architecture:** `OOP_WRAPPER_README.md` - ภาพรวมระบบ
3. **Deep dive:** `OOP_TECHNICAL_DESIGN.md` - รายละเอียดเทคนิค
4. **Examples:** `example_oop.c` - Code ตัวอย่างเต็มรูปแบบ

### External Resources

- [LVGL Documentation](https://docs.lvgl.io/)
- [MFC Programming](https://docs.microsoft.com/en-us/cpp/mfc/)
- [OOP in C](https://www.state-machine.com/doc/AN_OOP_in_C.pdf)

## 🚀 Future Enhancements

### Planned Features

- [ ] **More Widgets**
  - LVSlider - Slider control
  - LVCheckbox - Checkbox control
  - LVDropdown - Dropdown list
  - LVTextarea - Text input

- [ ] **Advanced Features**
  - Widget Templates (like MFC resource templates)
  - Visual Designer support
  - Animation helpers
  - Theme system

- [ ] **Developer Tools**
  - Widget inspector
  - Memory leak detector
  - Performance profiler

### How to Contribute

1. สร้าง derived class ใหม่ใน `include/` และ `*.c`
2. เพิ่มลงใน `CMakeLists.txt`
3. สร้าง example ใน `example_oop.c`
4. อัพเดท documentation

## 🐛 Known Issues

1. **IntelliSense Errors** - VS Code อาจแสดง include errors (ไม่กระทบการ compile)
2. **Downcast Safety** - ไม่มี runtime type checking (ใช้ระวัง!)
3. **Memory Leaks** - ต้อง destroy widgets เอง (หรือ destroy parent)

## 💡 Best Practices

1. **Always Check NULL**
   ```c
   if (!widget) return;
   ```

2. **Use Type-Safe Macros**
   ```c
   LVBUTTON_TO_WIDGET(btn)  // Not (LVWidget*)btn
   ```

3. **Lock LVGL Access**
   ```c
   bsp_display_lock(0);
   create_ui();
   bsp_display_unlock();
   ```

4. **Destroy Properly**
   ```c
   lvpanel_destroy(panel);  // Children destroyed automatically
   ```

## 📞 Contact & Support

- **Author:** LVGL OOP Wrapper Team
- **Version:** 1.0
- **Date:** 2026-01-12
- **License:** MIT (same as LVGL)

---

## 🎉 Conclusion

ระบบ OOP Wrapper นี้นำเอาความสะดวกของ MFC มาใช้กับ LVGL ทำให้:

- ✅ Code อ่านง่ายขึ้น (เหมือน C++ OOP)
- ✅ Type-safe มากขึ้น (ลด bugs)
- ✅ Reusable มากขึ้น (สร้าง widget ซ้ำได้ง่าย)
- ✅ Maintainable มากขึ้น (แก้ไขง่าย)

**Happy Coding with LVGL OOP! 🚀**
