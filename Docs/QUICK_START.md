# Quick Start Guide - LVGL OOP Wrapper

## 🚀 เริ่มต้นใช้งาน

### 1. Include Headers

```c
#include "include/lvgl_button.h"
#include "include/lvgl_label.h"
#include "include/lvgl_panel.h"
```

### 2. สร้าง UI แบบง่าย

```c
void create_simple_ui(void)
{
    // สร้าง Button
    LVButton* btn = lvbutton_create(NULL);
    lvbutton_set_text(btn, "Click Me");
    lvwidget_set_size(LVBUTTON_TO_WIDGET(btn), 150, 60);
    lvwidget_set_align(LVBUTTON_TO_WIDGET(btn), LV_ALIGN_CENTER, 0, 0);
    
    // ตั้ง event handler
    lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn), on_button_clicked);
}

// Event handler function
void on_button_clicked(LVWidget* widget, lv_event_t* event)
{
    ESP_LOGI("UI", "Button clicked!");
}
```

### 3. สร้าง Counter Example (แบบเต็ม)

```c
// Global variables
static LVLabel* lbl_counter;
static int counter = 0;

void create_counter_ui(void)
{
    // 1. Panel
    LVPanel* panel = lvpanel_create(NULL);
    lvwidget_set_size(LVPANEL_TO_WIDGET(panel), 400, 300);
    lvwidget_set_align(LVPANEL_TO_WIDGET(panel), LV_ALIGN_CENTER, 0, 0);
    lvpanel_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lvpanel_set_gap(panel, 20);
    
    // 2. Title Label
    LVLabel* lbl_title = lvlabel_create(LVPANEL_TO_WIDGET(panel));
    lvlabel_set_text(lbl_title, "Counter");
    lvlabel_set_font(lbl_title, &lv_font_montserrat_32);
    
    // 3. Counter Label
    lbl_counter = lvlabel_create(LVPANEL_TO_WIDGET(panel));
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
    lvlabel_set_font(lbl_counter, &lv_font_montserrat_48);
    
    // 4. Increment Button
    LVButton* btn_inc = lvbutton_create(LVPANEL_TO_WIDGET(panel));
    lvbutton_set_text(btn_inc, "+");
    lvwidget_set_size(LVBUTTON_TO_WIDGET(btn_inc), 120, 50);
    lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn_inc), on_increment);
}

void on_increment(LVWidget* widget, lv_event_t* event)
{
    counter++;
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
}
```

## 📖 Widget Reference

### LVButton

```c
// สร้าง
LVButton* btn = lvbutton_create(parent);

// ตั้งข้อความ
lvbutton_set_text(btn, "Click Me");

// ตั้ง font
lvbutton_set_font(btn, &lv_font_montserrat_20);

// ตั้งสีข้อความ
lvbutton_set_text_color(btn, lv_color_hex(0xFF0000));

// Common properties (ใช้กับทุก widget)
lvwidget_set_size(LVBUTTON_TO_WIDGET(btn), 150, 60);
lvwidget_set_pos(LVBUTTON_TO_WIDGET(btn), 100, 50);
lvwidget_set_align(LVBUTTON_TO_WIDGET(btn), LV_ALIGN_CENTER, 0, 0);
lvwidget_set_bg_color(LVBUTTON_TO_WIDGET(btn), lv_color_hex(0x2196F3));

// Event handlers
lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn), handler);
```

### LVLabel

```c
// สร้าง
LVLabel* lbl = lvlabel_create(parent);

// ตั้งข้อความ
lvlabel_set_text(lbl, "Hello World");

// ตั้งข้อความแบบ format
lvlabel_set_text_fmt(lbl, "Value: %d", 42);

// ตั้ง font
lvlabel_set_font(lbl, &lv_font_montserrat_32);

// ตั้งสี
lvlabel_set_text_color(lbl, lv_color_hex(0x00FF00));

// Long text mode
lvlabel_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
```

### LVPanel

```c
// สร้าง
LVPanel* panel = lvpanel_create(parent);

// ตั้ง layout
lvpanel_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);  // หรือ LV_FLEX_FLOW_ROW

// ตั้ง padding
lvpanel_set_padding(panel, 20);

// ตั้ง gap ระหว่าง items
lvpanel_set_gap(panel, 10);

// Enable scrolling
lvpanel_set_scrollable(panel, true);
```

## 🎨 Style Examples

### สี

```c
// Background
lvwidget_set_bg_color(widget, lv_color_hex(0x2196F3));

// Border
lvwidget_set_border_color(widget, lv_color_hex(0xFF0000));
lvwidget_set_border_width(widget, 2);

// Radius (มุมโค้ง)
lvwidget_set_radius(widget, 10);
```

### Fonts

```c
// Built-in fonts
&lv_font_montserrat_14   // Small
&lv_font_montserrat_20   // Medium
&lv_font_montserrat_32   // Large
&lv_font_montserrat_48   // Extra Large
```

### Alignment

```c
// Center
lvwidget_set_align(widget, LV_ALIGN_CENTER, 0, 0);

// Top-Left
lvwidget_set_align(widget, LV_ALIGN_TOP_LEFT, 10, 10);

// Bottom-Right
lvwidget_set_align(widget, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

// Top-Mid (with offset)
lvwidget_set_align(widget, LV_ALIGN_TOP_MID, 0, 20);
```

## 🎯 Event Handling

### Basic Event Handler

```c
void my_event_handler(LVWidget* widget, lv_event_t* event)
{
    ESP_LOGI("EVENT", "Event triggered!");
    
    // ทำอะไรก็ได้ที่นี่
}

// Set handler
lvwidget_set_on_clicked(widget, my_event_handler);
```

### Event Handler with Data

```c
typedef struct {
    int count;
    LVLabel* label;
} AppData;

void increment_handler(LVWidget* widget, lv_event_t* event)
{
    // ดึง user data
    AppData* data = (AppData*)lvwidget_get_user_data(widget);
    
    data->count++;
    lvlabel_set_text_fmt(data->label, "Count: %d", data->count);
}

// Set user data
AppData app_data = {0, lbl_counter};
lvwidget_set_user_data(LVBUTTON_TO_WIDGET(btn), &app_data);
lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn), increment_handler);
```

## 📦 Layout Examples

### Vertical Layout (Column)

```c
LVPanel* panel = lvpanel_create(NULL);
lvpanel_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
lvpanel_set_gap(panel, 10);

// เพิ่ม widgets (จะเรียงจากบนลงล่าง)
LVButton* btn1 = lvbutton_create(LVPANEL_TO_WIDGET(panel));
LVButton* btn2 = lvbutton_create(LVPANEL_TO_WIDGET(panel));
LVButton* btn3 = lvbutton_create(LVPANEL_TO_WIDGET(panel));
```

### Horizontal Layout (Row)

```c
LVPanel* panel = lvpanel_create(NULL);
lvpanel_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
lvpanel_set_gap(panel, 10);

// เพิ่ม widgets (จะเรียงจากซ้ายไปขวา)
LVButton* btn1 = lvbutton_create(LVPANEL_TO_WIDGET(panel));
LVButton* btn2 = lvbutton_create(LVPANEL_TO_WIDGET(panel));
LVButton* btn3 = lvbutton_create(LVPANEL_TO_WIDGET(panel));
```

## 🎨 Complete Examples

### Example 1: Simple Alert Dialog

```c
void create_alert_dialog(const char* message)
{
    // Background overlay
    LVPanel* overlay = lvpanel_create(NULL);
    lvwidget_set_size(LVPANEL_TO_WIDGET(overlay), 800, 480);
    lvwidget_set_bg_color(LVPANEL_TO_WIDGET(overlay), lv_color_hex(0x000000));
    // Set opacity to 50% (ใช้ LVGL API โดยตรง)
    lv_obj_set_style_bg_opa(overlay->base.obj, LV_OPA_50, 0);
    
    // Dialog panel
    LVPanel* dialog = lvpanel_create(LVPANEL_TO_WIDGET(overlay));
    lvwidget_set_size(LVPANEL_TO_WIDGET(dialog), 400, 200);
    lvwidget_set_align(LVPANEL_TO_WIDGET(dialog), LV_ALIGN_CENTER, 0, 0);
    lvwidget_set_bg_color(LVPANEL_TO_WIDGET(dialog), lv_color_hex(0xFFFFFF));
    lvpanel_set_flex_flow(dialog, LV_FLEX_FLOW_COLUMN);
    
    // Message
    LVLabel* lbl = lvlabel_create(LVPANEL_TO_WIDGET(dialog));
    lvlabel_set_text(lbl, message);
    lvlabel_set_font(lbl, &lv_font_montserrat_20);
    
    // OK button
    LVButton* btn = lvbutton_create(LVPANEL_TO_WIDGET(dialog));
    lvbutton_set_text(btn, "OK");
    lvwidget_set_size(LVBUTTON_TO_WIDGET(btn), 100, 40);
}
```

### Example 2: Settings Panel

```c
void create_settings_panel(void)
{
    LVPanel* main = lvpanel_create(NULL);
    lvwidget_set_size(LVPANEL_TO_WIDGET(main), 600, 400);
    lvwidget_set_align(LVPANEL_TO_WIDGET(main), LV_ALIGN_CENTER, 0, 0);
    lvpanel_set_flex_flow(main, LV_FLEX_FLOW_COLUMN);
    lvpanel_set_gap(main, 15);
    
    // Title
    LVLabel* title = lvlabel_create(LVPANEL_TO_WIDGET(main));
    lvlabel_set_text(title, "Settings");
    lvlabel_set_font(title, &lv_font_montserrat_32);
    
    // WiFi Setting
    create_setting_row(main, "WiFi", "Enabled");
    
    // Bluetooth Setting
    create_setting_row(main, "Bluetooth", "Disabled");
    
    // Brightness Setting
    create_setting_row(main, "Brightness", "80%");
}

void create_setting_row(LVPanel* parent, const char* name, const char* value)
{
    LVPanel* row = lvpanel_create(LVPANEL_TO_WIDGET(parent));
    lvpanel_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lvwidget_set_size(LVPANEL_TO_WIDGET(row), 550, 60);
    
    LVLabel* lbl_name = lvlabel_create(LVPANEL_TO_WIDGET(row));
    lvlabel_set_text(lbl_name, name);
    lvlabel_set_font(lbl_name, &lv_font_montserrat_20);
    
    LVLabel* lbl_value = lvlabel_create(LVPANEL_TO_WIDGET(row));
    lvlabel_set_text(lbl_value, value);
    lvlabel_set_font(lbl_value, &lv_font_montserrat_20);
}
```

## ⚠️ Common Pitfalls

### 1. Forget to Cast

```c
// ❌ Wrong
lvbutton_set_text(btn, "Text");
lvwidget_set_size(btn, 100, 50);  // Error! Type mismatch

// ✅ Correct
lvbutton_set_text(btn, "Text");
lvwidget_set_size(LVBUTTON_TO_WIDGET(btn), 100, 50);
```

### 2. Destroy Children Manually

```c
// ❌ Not necessary (parent will destroy children)
lvbutton_destroy(btn1);
lvbutton_destroy(btn2);
lvpanel_destroy(panel);

// ✅ Just destroy parent
lvpanel_destroy(panel);  // จะลบ children อัตโนมัติ
```

### 3. NULL Check

```c
// ❌ May crash
void update_label(LVLabel* lbl, int value) {
    lvlabel_set_text_fmt(lbl, "Value: %d", value);
}

// ✅ Safe
void update_label(LVLabel* lbl, int value) {
    if (!lbl) return;
    lvlabel_set_text_fmt(lbl, "Value: %d", value);
}
```

## 📚 Next Steps

1. ดูไฟล์ `example_oop.c` สำหรับตัวอย่างเต็มรูปแบบ
2. อ่าน `OOP_WRAPPER_README.md` สำหรับรายละเอียดเพิ่มเติม
3. อ่าน `OOP_TECHNICAL_DESIGN.md` สำหรับ implementation details

## 🎯 Tips

1. **ใช้ LVGL Lock** - ห่อ UI code ด้วย `bsp_display_lock()`/`bsp_display_unlock()`
2. **Global Variables** - สำหรับ widgets ที่ต้องเข้าถึงจาก event handlers
3. **User Data** - ใช้ `lvwidget_set_user_data()` สำหรับ context data
4. **Naming Convention** - ใช้ `lvwidget_*`, `lvbutton_*`, `lvlabel_*`, `lvpanel_*`

Happy Coding! 🎉
