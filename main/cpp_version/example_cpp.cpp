#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "../include/LVButton.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVPanel.hpp"

/**
 * @brief Modern C++ LVGL Example
 * 
 * ตัวอย่างนี้แสดงการใช้งาน Modern C++ features:
 * - Classes และ Inheritance
 * - Lambda functions
 * - std::function
 * - RAII pattern
 * - Method chaining (fluent API)
 * - Template functions
 */

// using namespace lvgl;

static const char* TAG = "CPP_Example";

// ===== Global Widgets (หรือจะใช้ smart pointers ก็ได้) =====

static LVPanel* g_mainPanel = nullptr;
static LVButton* g_btnIncrement = nullptr;
static LVButton* g_btnDecrement = nullptr;
static LVButton* g_btnReset = nullptr;
static LVLabel* g_lblTitle = nullptr;
static LVLabel* g_lblCounter = nullptr;

// Application state
// static int g_counter = 0;

// ===== Modern C++ Style - Class-Based Application =====

/**
 * @brief Counter Application Class (แบบ Qt/WPF)
 * 
 * ใช้ class เพื่อ encapsulate UI และ logic
 */
class CounterApp {
public:
    CounterApp() : m_counter(0) {
        ESP_LOGI(TAG, "Creating Counter App...");
        createUI();
    }
    
    ~CounterApp() {
        ESP_LOGI(TAG, "Destroying Counter App...");
        cleanup();
    }
    
private:
    void createUI() {
        // สร้าง Main Panel พร้อม method chaining (fluent API)
        m_mainPanel = new LVPanel();
        m_mainPanel->setSize(600, 400)
                   ->setAlign(LV_ALIGN_CENTER, 0, 0)
                   ->setBackgroundColor(lv_color_hex(0x2C3E50));
        m_mainPanel->withFlexFlow(LV_FLEX_FLOW_COLUMN)
                   ->withGap(20)
                   ->withPadding(30);
        
        // สร้าง Title Label (fluent API)
        m_lblTitle = new LVLabel(m_mainPanel);
        m_lblTitle->withText("C++ Counter Demo")
                  ->withFont(&lv_font_montserrat_32)
                  ->withColor(lv_color_hex(0xECF0F1));
        m_lblTitle->setAlign(LV_ALIGN_TOP_MID, 0, 0);
        
        // สร้าง Counter Label (template function สำหรับ formatting)
        m_lblCounter = new LVLabel(m_mainPanel);
        updateCounter();  // Set initial text
        m_lblCounter->withFont(&lv_font_montserrat_48)
                    ->withColor(lv_color_hex(0x3498DB));
        m_lblCounter->setAlign(LV_ALIGN_CENTER, 0, -20);
        
        // สร้าง Button Panel
        auto* btnPanel = new LVPanel(m_mainPanel);
        btnPanel->setSize(500, 80);
        btnPanel->withFlexFlow(LV_FLEX_FLOW_ROW)
               ->withGap(15)
               ->setBackgroundColor(lv_color_hex(0x34495E));
        btnPanel->setAlign(LV_ALIGN_BOTTOM_MID, 0, -20);
        
        // สร้าง Increment Button (Lambda event handler!)
        m_btnIncrement = new LVButton(btnPanel);
        m_btnIncrement->withText("+")
                     ->withFont(&lv_font_montserrat_32);
        m_btnIncrement->setSize(140, 60);
        m_btnIncrement->setBackgroundColor(lv_color_hex(0x27AE60));
        
        // Modern C++ Lambda! 🎉
        m_btnIncrement->onClicked([this](auto* widget, auto* event) {
            this->onIncrementClicked();
        });
        
        // สร้าง Reset Button
        m_btnReset = new LVButton(btnPanel);
        m_btnReset->withText("Reset")
                  ->withFont(&lv_font_montserrat_20);
        m_btnReset->setSize(140, 60);
        m_btnReset->setBackgroundColor(lv_color_hex(0xE67E22));
        
        // Lambda with capture
        m_btnReset->onClicked([this](LVWidget*, lv_event_t*) {
            this->onResetClicked();
        });
        
        // สร้าง Decrement Button
        m_btnDecrement = new LVButton(btnPanel);
        m_btnDecrement->withText("-")
                     ->withFont(&lv_font_montserrat_32);
        m_btnDecrement->setSize(140, 60);
        m_btnDecrement->setBackgroundColor(lv_color_hex(0xE74C3C));
        
        // Lambda
        m_btnDecrement->onClicked([this](auto*, auto*) {
            this->onDecrementClicked();
        });
        
        ESP_LOGI(TAG, "UI Created!");
    }
    
    void cleanup() {
        // Delete ในลำดับกลับกัน (แต่จริงๆ delete parent ก็พอ)
        delete m_mainPanel;  // จะลบ children อัตโนมัติ
        m_mainPanel = nullptr;
    }
    
    void updateCounter() {
        // ใช้ template function สำหรับ formatting
        m_lblCounter->setTextFormat("Count: ", m_counter);
    }
    
    // Event handlers (private methods)
    void onIncrementClicked() {
        m_counter++;
        ESP_LOGI(TAG, "Increment! Counter = %d", m_counter);
        updateCounter();
    }
    
    void onDecrementClicked() {
        m_counter--;
        ESP_LOGI(TAG, "Decrement! Counter = %d", m_counter);
        updateCounter();
    }
    
    void onResetClicked() {
        m_counter = 0;
        ESP_LOGI(TAG, "Reset! Counter = %d", m_counter);
        updateCounter();
    }
    
    // Member variables
    int m_counter;
    LVPanel* m_mainPanel;
    LVButton* m_btnIncrement;
    LVButton* m_btnDecrement;
    LVButton* m_btnReset;
    LVLabel* m_lblTitle;
    LVLabel* m_lblCounter;
};

// Global app instance
static CounterApp* g_app = nullptr;

// ===== Alternative: Function-based approach with lambdas =====

void example_cpp_functional_style()
{
    ESP_LOGI(TAG, "Creating Functional Style UI...");
    
    // สร้าง Panel
    auto* panel = new LVPanel();
    panel->setSize(400, 300)
         ->setAlign(LV_ALIGN_CENTER, 0, 0)
         ->setBackgroundColor(lv_color_hex(0x34495E));
    panel->withFlexFlow(LV_FLEX_FLOW_COLUMN)
         ->withPadding(20);
    
    // สร้าง Label
    auto* label = new LVLabel(panel);
    label->setText("Hello, Modern C++!");
    label->setFont(&lv_font_montserrat_32);
    
    // สร้าง Button พร้อม Lambda
    auto* button = new LVButton(panel);
    button->setText("Click Me!");
    button->setSize(200, 60);
    
    // Lambda with local state capture
    static int clickCount = 0;
    button->onClicked([label](auto*, auto*) {
        clickCount++;
        label->setTextFormat("Clicked ", clickCount, " times!");
        ESP_LOGI(TAG, "Button clicked %d times", clickCount);
    });
}

// ===== Alternative: Custom Widget (Inheritance) =====

/**
 * @brief Custom Widget Example - Spin Box
 * 
 * สร้าง custom widget โดย inherit จาก LVPanel
 */
class SpinBox : public LVPanel {
public:
    SpinBox(LVWidget* parent = nullptr, int minVal = 0, int maxVal = 100)
        : LVPanel(parent)
        , m_value(minVal)
        , m_minValue(minVal)
        , m_maxValue(maxVal)
    {
        createUI();
    }
    
    int value() const { return m_value; }
    
    void setValue(int val) {
        m_value = std::max(m_minValue, std::min(m_maxValue, val));
        updateDisplay();
    }
    
    // Signal-like callback (Qt style)
    void onValueChanged(std::function<void(int)> callback) {
        m_valueChangedCallback = callback;
    }
    
private:
    void createUI() {
        setSize(200, 60);
        withFlexFlow(LV_FLEX_FLOW_ROW)->withGap(10);
        
        // Decrement button
        m_btnDec = new LVButton(this);
        m_btnDec->setText("-")->setSize(50, 50);
        m_btnDec->onClicked([this](auto*, auto*) {
            this->decrement();
        });
        
        // Value label
        m_lblValue = new LVLabel(this);
        m_lblValue->setFont(&lv_font_montserrat_32);
        updateDisplay();
        
        // Increment button
        m_btnInc = new LVButton(this);
        m_btnInc->setText("+")->setSize(50, 50);
        m_btnInc->onClicked([this](auto*, auto*) {
            this->increment();
        });
    }
    
    void increment() {
        if (m_value < m_maxValue) {
            m_value++;
            updateDisplay();
            notifyValueChanged();
        }
    }
    
    void decrement() {
        if (m_value > m_minValue) {
            m_value--;
            updateDisplay();
            notifyValueChanged();
        }
    }
    
    void updateDisplay() {
        m_lblValue->setTextFormat(m_value);
    }
    
    void notifyValueChanged() {
        if (m_valueChangedCallback) {
            m_valueChangedCallback(m_value);
        }
    }
    
    int m_value;
    int m_minValue;
    int m_maxValue;
    LVButton* m_btnInc;
    LVButton* m_btnDec;
    LVLabel* m_lblValue;
    std::function<void(int)> m_valueChangedCallback;
};

void example_custom_widget()
{
    ESP_LOGI(TAG, "Creating Custom Widget Example...");
    
    auto* panel = new LVPanel();
    panel->setSize(400, 300)->setAlign(LV_ALIGN_CENTER, 0, 0);
    panel->withFlexFlow(LV_FLEX_FLOW_COLUMN)->withGap(20);
    
    auto* label = new LVLabel(panel);
    label->setText("Spin Box Demo");
    label->setFont(&lv_font_montserrat_24);
    
    // สร้าง custom SpinBox widget
    auto* spinBox = new SpinBox(panel, 0, 100);
    spinBox->setValue(50);
    
    // ผลลัพธ์ label
    auto* resultLabel = new LVLabel(panel);
    resultLabel->setFont(&lv_font_montserrat_20);
    
    // เชื่อมต่อ signal (Qt-style)
    spinBox->onValueChanged([resultLabel](int value) {
        resultLabel->setTextFormat("Value changed to: ", value);
        ESP_LOGI(TAG, "SpinBox value: %d", value);
    });
}

// ===== Main Entry Points =====

extern "C" void test_cpp_example()
{
    ESP_LOGI(TAG, "Starting C++ OOP Example...");
    
    // สร้าง application (RAII - จะ cleanup อัตโนมัติเมื่อ destroy)
    g_app = new CounterApp();
    
    ESP_LOGI(TAG, "C++ App ready!");
}

extern "C" void test_cpp_functional()
{
    example_cpp_functional_style();
}

extern "C" void test_cpp_custom_widget()
{
    example_custom_widget();
}

// ===== Cleanup =====

extern "C" void cleanup_cpp_example()
{
    if (g_app) {
        delete g_app;
        g_app = nullptr;
    }
}
