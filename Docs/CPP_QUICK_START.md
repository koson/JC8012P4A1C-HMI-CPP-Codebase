# LVGL C++ OOP Wrapper - Quick Start Guide

## 🚀 เริ่มต้นใช้งานภายใน 5 นาที!

### ข้อกำหนดเบื้องต้น

- ✅ ESP-IDF v5.0+ (รองรับ C++17)
- ✅ LVGL v9.x
- ✅ VSCode + ESP-IDF Extension

---

## 📦 1. Setup Project

### Enable C++ Support

แก้ไข `CMakeLists.txt` ในโฟลเดอร์ `main/`:

```cmake
# เพิ่มการรองรับ C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(
    SRCS 
        # C++ files
        LVWidget.cpp
        LVButton.cpp
        LVLabel.cpp
        LVPanel.cpp
        example_cpp.cpp
        
        # C files (existing)
        main.c
        widgets.c
        
    INCLUDE_DIRS 
        . 
        include
)
```

---

## 💻 2. Your First C++ Widget

### Example 1: Simple Button

```cpp
#include "include/LVButton.hpp"

using namespace lvgl;

extern "C" void app_main(void)
{
    // ... initialize display ...
    
    bsp_display_lock(0);
    
    // สร้าง button (Modern C++!)
    auto* btn = new LVButton();
    
    // ตั้งค่าด้วย method chaining
    btn->setText("Click Me!")
       ->setSize(150, 60)
       ->setAlign(LV_ALIGN_CENTER, 0, 0)
       ->setBackgroundColor(lv_color_hex(0x27AE60));
    
    // Event handler ด้วย Lambda! 🎉
    btn->onClicked([](auto* widget, auto* event) {
        ESP_LOGI("APP", "Button clicked!");
    });
    
    bsp_display_unlock();
}
```

**Output:**
```
I (1234) APP: Button clicked!
I (1235) APP: Button clicked!
```

---

### Example 2: Counter App

```cpp
#include "include/LVPanel.hpp"
#include "include/LVButton.hpp"
#include "include/LVLabel.hpp"

using namespace lvgl;

extern "C" void app_main(void)
{
    // ... initialize display ...
    
    bsp_display_lock(0);
    
    // สร้าง Panel (Container)
    auto* panel = new LVPanel();
    panel->setSize(400, 300)
         ->setAlign(LV_ALIGN_CENTER, 0, 0)
         ->withFlexFlow(LV_FLEX_FLOW_COLUMN)
         ->withGap(20);
    
    // สร้าง Label สำหรับแสดงตัวเลข
    auto* label = new LVLabel(panel);
    label->setText("Count: 0")
         ->setFont(&lv_font_montserrat_32);
    
    // Local state (captured by lambda)
    static int counter = 0;
    
    // สร้าง Button เพิ่มเลข
    auto* btnInc = new LVButton(panel);
    btnInc->setText("+")
          ->setSize(100, 50);
    
    // Lambda with capture!
    btnInc->onClicked([label](auto*, auto*) {
        counter++;
        label->setTextFormat("Count: ", counter);  // Template function!
    });
    
    // สร้าง Button ลดเลข
    auto* btnDec = new LVButton(panel);
    btnDec->setText("-")
          ->setSize(100, 50);
    
    btnDec->onClicked([label](auto*, auto*) {
        counter--;
        label->setTextFormat("Count: ", counter);
    });
    
    bsp_display_unlock();
}
```

---

### Example 3: Class-Based App (MFC/Qt Style)

```cpp
class MyApp {
public:
    MyApp() {
        createUI();
    }
    
    ~MyApp() {
        delete m_panel;  // Automatic cleanup of children
    }
    
private:
    void createUI() {
        m_panel = new LVPanel();
        m_panel->setSize(600, 400)
               ->setAlign(LV_ALIGN_CENTER, 0, 0);
        
        m_label = new LVLabel(m_panel);
        m_label->setText("Hello, C++!");
        
        m_button = new LVButton(m_panel);
        m_button->setText("Click Me");
        
        // Bind to member function
        m_button->onClicked([this](auto*, auto*) {
            this->onButtonClicked();
        });
    }
    
    void onButtonClicked() {
        m_clickCount++;
        m_label->setTextFormat("Clicked ", m_clickCount, " times!");
    }
    
    LVPanel* m_panel;
    LVLabel* m_label;
    LVButton* m_button;
    int m_clickCount = 0;
};

// Global instance
static MyApp* g_app = nullptr;

extern "C" void app_main(void)
{
    // ... initialize ...
    
    bsp_display_lock(0);
    g_app = new MyApp();  // RAII!
    bsp_display_unlock();
}
```

---

## 🎨 3. Available Widgets

### LVWidget (Base Class)

```cpp
auto* widget = new LVWidget();

// Properties
widget->setSize(200, 100);
widget->setPos(50, 50);
widget->setAlign(LV_ALIGN_CENTER, 0, 0);
widget->show();
widget->hide();
widget->enable();
widget->disable();

// Styles (fluent API)
widget->setBackgroundColor(lv_color_hex(0xFF0000))
      ->setBorderColor(lv_color_hex(0x000000))
      ->setBorderWidth(2)
      ->setRadius(10);

// Events
widget->onClicked([](auto* w, auto* e) {
    ESP_LOGI("UI", "Widget clicked");
});
```

### LVButton

```cpp
auto* btn = new LVButton();

btn->setText("Click Me");
btn->setFont(&lv_font_montserrat_20);
btn->setTextColor(lv_color_white());

// Fluent API
btn->withText("Button")
   ->withFont(&lv_font_montserrat_24);

// Event
btn->onClicked([](auto*, auto*) {
    ESP_LOGI("UI", "Button clicked!");
});
```

### LVLabel

```cpp
auto* label = new LVLabel();

label->setText("Hello");
label->setText(std::string("World"));

// Template formatting (C++17)
int value = 42;
label->setTextFormat("Value: ", value);  // "Value: 42"
label->setTextFormat("X: ", x, ", Y: ", y);  // Multiple args!

// Fluent API
label->withText("Title")
     ->withFont(&lv_font_montserrat_32)
     ->withColor(lv_color_hex(0x00FF00));
```

### LVPanel

```cpp
auto* panel = new LVPanel();

panel->setFlexFlow(LV_FLEX_FLOW_COLUMN);
panel->setPadding(20);
panel->setGap(10);
panel->setScrollable(true);

// Fluent API
panel->withFlexFlow(LV_FLEX_FLOW_ROW)
     ->withPadding(15)
     ->withGap(5);

// Children
auto* child1 = new LVButton(panel);
auto* child2 = new LVLabel(panel);
```

---

## 🔧 4. Advanced Features

### Custom Widget

```cpp
class SpinBox : public LVPanel {
public:
    SpinBox(int min = 0, int max = 100) 
        : LVPanel()
        , m_value(min)
        , m_min(min)
        , m_max(max)
    {
        createUI();
    }
    
    int value() const { return m_value; }
    void setValue(int v) { 
        m_value = std::clamp(v, m_min, m_max);
        updateDisplay();
    }
    
    // Signal-like callback
    void onValueChanged(std::function<void(int)> cb) {
        m_callback = cb;
    }
    
private:
    void createUI() {
        setFlexFlow(LV_FLEX_FLOW_ROW);
        
        m_btnDec = new LVButton(this);
        m_btnDec->setText("-");
        m_btnDec->onClicked([this](auto*, auto*) {
            setValue(m_value - 1);
        });
        
        m_label = new LVLabel(this);
        updateDisplay();
        
        m_btnInc = new LVButton(this);
        m_btnInc->setText("+");
        m_btnInc->onClicked([this](auto*, auto*) {
            setValue(m_value + 1);
        });
    }
    
    void updateDisplay() {
        m_label->setTextFormat(m_value);
        if (m_callback) m_callback(m_value);
    }
    
    int m_value, m_min, m_max;
    LVButton *m_btnInc, *m_btnDec;
    LVLabel* m_label;
    std::function<void(int)> m_callback;
};

// Usage
auto* spin = new SpinBox(0, 100);
spin->setValue(50);
spin->onValueChanged([](int val) {
    ESP_LOGI("UI", "Value changed: %d", val);
});
```

---

## 📚 5. Common Patterns

### Pattern 1: Method Chaining

```cpp
// สร้างและตั้งค่าในบรรทัดเดียว
auto* btn = new LVButton();
btn->setText("OK")
   ->setSize(100, 50)
   ->setAlign(LV_ALIGN_CENTER, 0, 0)
   ->setBackgroundColor(lv_color_hex(0x27AE60));
```

### Pattern 2: Lambda Event Handlers

```cpp
// Inline lambda
btn->onClicked([](auto*, auto*) {
    ESP_LOGI("UI", "Clicked!");
});

// Capture local variables
int counter = 0;
btn->onClicked([&counter](auto*, auto*) {
    counter++;  // Modify captured variable
});

// Capture this
btn->onClicked([this](auto*, auto*) {
    this->handleClick();
});
```

### Pattern 3: RAII Resource Management

```cpp
void createDialog() {
    auto* dialog = new LVPanel();
    auto* btn = new LVButton(dialog);
    
    // No need to manually delete children!
    // delete btn;  // ← ไม่ต้อง!
    
    delete dialog;  // จะลบ btn อัตโนมัติ
}

// Or use smart pointers (C++14)
std::unique_ptr<LVPanel> dialog(new LVPanel());
// Automatic cleanup when out of scope!
```

### Pattern 4: Template Formatting

```cpp
auto* label = new LVLabel();

// Old way (C-style)
char buf[64];
snprintf(buf, sizeof(buf), "X: %d, Y: %d", x, y);
label->setText(buf);

// New way (C++ template)
label->setTextFormat("X: ", x, ", Y: ", y);  // Type-safe!
```

---

## ⚙️ 6. Build & Run

### Build Project

```bash
# Terminal
idf.py build

# Or in VS Code
# Ctrl+Shift+P -> ESP-IDF: Build Project
```

### Flash to Device

```bash
idf.py -p COM3 flash monitor
```

### Expected Output

```
I (1234) LVWidget: Widget created: 0x3ffc1000
I (1235) LVButton: Button created: 0x3ffc1100 (text: Click Me!)
I (1236) LVLabel: Label created: 0x3ffc1200 (text: Count: 0)
I (5678) UI: Button clicked!
I (5679) LVButton: Button clicked: 0x3ffc1100 (text: +)
```

---

## 🐛 7. Common Issues

### Issue 1: Compiler Errors

```
error: 'std::string' is not a member of 'std'
```

**Solution:** Enable C++17 in CMakeLists.txt:
```cmake
set(CMAKE_CXX_STANDARD 17)
```

### Issue 2: Linker Errors

```
undefined reference to `LVButton::LVButton()'
```

**Solution:** Add .cpp files to CMakeLists.txt:
```cmake
idf_component_register(
    SRCS LVButton.cpp LVLabel.cpp ...
)
```

### Issue 3: Include Errors

```
fatal error: LVButton.hpp: No such file or directory
```

**Solution:** Add include directory:
```cmake
INCLUDE_DIRS . include
```

---

## 📖 8. Next Steps

1. ✅ **สำรวจ Examples:** ดูที่ [example_cpp.cpp](main/example_cpp.cpp)
2. ✅ **เรียนรู้ Patterns:** อ่าน [CPP_VS_C_COMPARISON.md](CPP_VS_C_COMPARISON.md)
3. ✅ **สร้าง Custom Widgets:** Inherit จาก LVWidget/LVPanel
4. ✅ **ใช้ Smart Pointers:** ลองใช้ `std::unique_ptr`

---

## 💡 9. Tips & Best Practices

### ✅ DO's

```cpp
// ✅ ใช้ auto เพื่อความสะดวก
auto* btn = new LVButton();

// ✅ ใช้ lambda สำหรับ event handlers
btn->onClicked([](auto*, auto*) { /* ... */ });

// ✅ ใช้ method chaining
panel->withFlexFlow(LV_FLEX_FLOW_COLUMN)->withGap(10);

// ✅ ใช้ RAII pattern
{
    auto* temp = new LVPanel();
    // ...
    delete temp;  // หรือใช้ smart pointer
}

// ✅ Capture by reference สำหรับ local variables
int count = 0;
btn->onClicked([&count](auto*, auto*) { count++; });
```

### ❌ DON'Ts

```cpp
// ❌ ลืม delete (memory leak)
auto* btn = new LVButton();
// ลืม: delete btn;

// ❌ Delete children manually (parent will do it)
auto* panel = new LVPanel();
auto* btn = new LVButton(panel);
delete btn;  // ❌ ไม่ต้อง! panel จะลบให้

// ❌ Use raw char* แทน std::string
char* text = "Hello";  // ❌
std::string text = "Hello";  // ✅

// ❌ Capture by value ถ้าต้องการ modify
int count = 0;
btn->onClicked([count](auto*, auto*) { 
    count++;  // ❌ ไม่มีผล! (copy)
});
// Fix: [&count]
```

---

## 🎉 Ready to Go!

คุณพร้อมแล้วที่จะสร้าง UI แบบ Modern C++ กับ LVGL!

**Quick Reference:**
- [LVWidget.hpp](main/include/LVWidget.hpp) - Base class API
- [LVButton.hpp](main/include/LVButton.hpp) - Button API
- [LVLabel.hpp](main/include/LVLabel.hpp) - Label API
- [LVPanel.hpp](main/include/LVPanel.hpp) - Panel API
- [example_cpp.cpp](main/example_cpp.cpp) - Complete examples

**Happy Coding! 🚀**
