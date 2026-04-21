# LVGL OOP Wrapper System

ระบบ OOP Wrapper สำหรับ LVGL v9 ที่ออกแบบตามแนวคิดของ MFC (Microsoft Foundation Classes)

## 📚 แนวคิด

เหมือนกับที่ MFC ห่อหุ้ม WIN32 API เป็น C++ Classes ระบบนี้ห่อหุ้ม LVGL Widgets เป็น OOP Pattern ใน C โดยใช้:

- **Inheritance** - ใช้ struct composition และ vtable
- **Polymorphism** - ใช้ function pointers ใน vtable
- **Encapsulation** - ซ่อน implementation details

## 🏗️ สถาปัตยกรรม

```
LVWidget (Base Class)
├── LVButton (Derived Class)
├── LVLabel (Derived Class)
└── LVPanel (Derived Class)
```

### Base Class: LVWidget

คล้าย `CWnd` ใน MFC - เป็น base class สำหรับ widget ทั้งหมด

**หลักการ:**
- Virtual Function Table (vtable) - เหมือน C++ vtable
- Properties struct - เก็บ state ของ widget
- Event handlers - เหมือน message map ใน MFC

**Methods:**
- `lvwidget_create()` - Constructor
- `lvwidget_destroy()` - Destructor (virtual)
- `lvwidget_set_size()` - ตั้งขนาด
- `lvwidget_set_pos()` - ตั้งตำแหน่ง
- `lvwidget_show()/hide()` - แสดง/ซ่อน
- `lvwidget_set_on_clicked()` - ตั้ง event handler

### Derived Class: LVButton

คล้าย `CButton` ใน MFC

**Methods:**
- `lvbutton_create()` - Constructor
- `lvbutton_set_text()` - ตั้งข้อความ (เหมือน SetWindowText)
- `lvbutton_set_font()` - ตั้ง font
- `lvbutton_set_text_color()` - ตั้งสีข้อความ

### Derived Class: LVLabel

คล้าย `CStatic` ใน MFC

**Methods:**
- `lvlabel_create()` - Constructor
- `lvlabel_set_text()` - ตั้งข้อความ
- `lvlabel_set_text_fmt()` - ตั้งข้อความแบบ printf
- `lvlabel_set_font()` - ตั้ง font

### Derived Class: LVPanel

คล้าง `CDialog` หรือ Panel ใน WinForms

**Methods:**
- `lvpanel_create()` - Constructor
- `lvpanel_set_flex_flow()` - ตั้ง layout
- `lvpanel_set_padding()` - ตั้ง padding
- `lvpanel_set_gap()` - ตั้งช่องว่างระหว่าง items

## 💻 ตัวอย่างการใช้งาน

### แบบ Simple

```c
// สร้าง Panel
LVPanel* panel = lvpanel_create(NULL);
lvwidget_set_size(LVPANEL_TO_WIDGET(panel), 600, 400);
lvwidget_set_align(LVPANEL_TO_WIDGET(panel), LV_ALIGN_CENTER, 0, 0);

// สร้าง Label
LVLabel* label = lvlabel_create(LVPANEL_TO_WIDGET(panel));
lvlabel_set_text(label, "Hello OOP!");
lvlabel_set_font(label, &lv_font_montserrat_32);

// สร้าง Button
LVButton* button = lvbutton_create(LVPANEL_TO_WIDGET(panel));
lvbutton_set_text(button, "Click Me");
lvwidget_set_size(LVBUTTON_TO_WIDGET(button), 120, 50);

// ตั้ง Event Handler (เหมือน ON_BN_CLICKED ใน MFC)
lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(button), on_button_clicked);
```

### แบบ MFC Dialog Style

```c
typedef struct {
    int counter;
    LVPanel* main_panel;
    LVButton* btn_increment;
    LVLabel* lbl_counter;
} CounterDialog;

// OnInitDialog
void counter_dialog_init(CounterDialog* dlg) {
    dlg->counter = 0;
    dlg->main_panel = lvpanel_create(NULL);
    dlg->btn_increment = lvbutton_create(LVPANEL_TO_WIDGET(dlg->main_panel));
    // ...
}

// UpdateData
void counter_dialog_update(CounterDialog* dlg) {
    lvlabel_set_text_fmt(dlg->lbl_counter, "Count: %d", dlg->counter);
}

// Event Handler
void on_increment_clicked(LVWidget* widget, lv_event_t* event) {
    g_dialog.counter++;
    counter_dialog_update(&g_dialog);  // UpdateData(FALSE)
}
```

## 🔧 หลักการทำงาน

### 1. Inheritance ใน C

```c
struct LVButton {
    LVWidget base;      // Base class (ต้องอยู่ตำแหน่งแรก!)
    lv_obj_t* label;    // Derived class members
    // ...
};

// Upcast (ปลอดภัย)
LVWidget* widget = LVBUTTON_TO_WIDGET(button);

// Downcast (ต้องระวัง - ต้องแน่ใจว่าเป็น type ที่ถูกต้อง)
LVButton* button = WIDGET_TO_LVBUTTON(widget);
```

### 2. Virtual Functions ด้วย VTable

```c
struct LVWidgetVTable {
    void (*destroy)(LVWidget* widget);
    void (*set_size)(LVWidget* widget, int32_t w, int32_t h);
    void (*on_clicked)(LVWidget* widget, lv_event_t* event);
    // ...
};

// Base class vtable
static const LVWidgetVTable default_vtable = {
    .destroy = default_destroy,
    .set_size = default_set_size,
    // ...
};

// Derived class vtable (override)
static const LVWidgetVTable button_vtable = {
    .destroy = button_destroy,      // Override
    .set_size = NULL,               // Use default
    .on_clicked = button_on_clicked,// Override
    // ...
};
```

### 3. Event Handling System

คล้าย Message Map ใน MFC:

```c
// 1. ตั้ง event handler
lvwidget_set_on_clicked(widget, my_handler);

// 2. LVGL event -> Dispatcher
void lvwidget_event_callback(lv_event_t* e) {
    LVWidget* widget = lv_obj_get_user_data(obj);
    
    // Dispatch to user handler
    if (widget->event_handlers.on_clicked) {
        widget->event_handlers.on_clicked(widget, e);
    }
    
    // Call virtual function
    if (widget->vtable->on_clicked) {
        widget->vtable->on_clicked(widget, e);
    }
}
```

## 📁 โครงสร้างไฟล์

```
main/
├── include/
│   ├── lvgl_widget.h      # Base class header
│   ├── lvgl_button.h      # Button class header
│   ├── lvgl_label.h       # Label class header
│   └── lvgl_panel.h       # Panel class header
├── lvgl_widget.c          # Base class implementation
├── lvgl_button.c          # Button implementation
├── lvgl_label.c           # Label implementation
├── lvgl_panel.c           # Panel implementation
└── example_oop.c          # ตัวอย่างการใช้งาน
```

## 🎯 ข้อดีของระบบนี้

1. **คุ้นเคย** - ใครที่เคยใช้ MFC/WinForms จะคุ้นเคยทันที
2. **Type Safety** - มากกว่าการใช้ LVGL API โดยตรง
3. **Encapsulation** - ซ่อน complexity ของ LVGL
4. **Reusable** - สร้าง widget ที่ซับซ้อนได้ง่าย
5. **Maintainable** - โค้ดอ่านง่าย จัดการง่าย

## 🚀 การขยายระบบ

### เพิ่ม Widget ใหม่

1. สร้าง header file ใน `include/`
2. สร้าง implementation file
3. Inherit จาก `LVWidget`
4. สร้าง vtable และ override methods ที่ต้องการ
5. เพิ่มลงใน CMakeLists.txt

### ตัวอย่าง: สร้าง LVSlider

```c
// lvgl_slider.h
typedef struct {
    LVWidget base;
    int32_t min_value;
    int32_t max_value;
    int32_t current_value;
} LVSlider;

LVSlider* lvslider_create(LVWidget* parent);
void lvslider_set_range(LVSlider* slider, int32_t min, int32_t max);
void lvslider_set_value(LVSlider* slider, int32_t value);
int32_t lvslider_get_value(LVSlider* slider);
```

## 🎓 เปรียบเทียบกับ MFC

| MFC | LVGL OOP Wrapper |
|-----|------------------|
| `CWnd` | `LVWidget` |
| `CButton` | `LVButton` |
| `CStatic` | `LVLabel` |
| `CDialog` | `LVPanel` |
| `SetWindowText()` | `lvbutton_set_text()` |
| `ON_BN_CLICKED` | `lvwidget_set_on_clicked()` |
| `UpdateData(FALSE)` | `counter_dialog_update()` |
| `virtual OnDestroy()` | `vtable->destroy()` |

## 📝 หมายเหตุ

- ใช้หลักการ OOP ใน C แบบ manual (ไม่มี compiler support เหมือน C++)
- Inheritance ทำด้วย struct composition (base class ต้องอยู่ตำแหน่งแรก!)
- Polymorphism ทำด้วย function pointers (vtable)
- Downcast ไม่ปลอดภัยเหมือน C++ dynamic_cast - ต้องระวัง!

## 🔗 อ้างอิง

- LVGL Documentation: https://docs.lvgl.io/
- MFC Programming: https://docs.microsoft.com/en-us/cpp/mfc/
- OOP in C: https://www.state-machine.com/doc/AN_OOP_in_C.pdf

---

**สร้างโดย:** ระบบ OOP Wrapper แบบ MFC-style
**Version:** 1.0
**Date:** 2026-01-12
