#ifndef LVWIDGET_HPP
#define LVWIDGET_HPP

#include "lvgl.h"
#include "esp_log.h"
#include <functional>
#include <memory>
#include <string>

/**
 * @brief Base Widget Class - C++ Version (คล้าย QWidget ใน Qt)
 * 
 * นี่คือ base class แบบ C++ แท้ๆ ที่ใช้:
 * - virtual functions แทน vtable
 * - constructor/destructor แทน create/destroy functions
 * - std::function แทน function pointers
 * - RAII pattern สำหรับ resource management
 */

// Forward declarations
class LVWidget;

/**
 * @brief Event handler type using std::function
 * รองรับ lambda, function objects, และ function pointers
 */
using EventHandler = std::function<void(LVWidget*, lv_event_t*)>;

/**
 * @brief Base Widget Class
 * 
 * เหมือน QWidget ใน Qt หรือ Control ใน WPF
 */
class LVWidget {
public:
    /**
     * @brief Constructor
     * @param parent Parent widget (nullptr สำหรับ root)
     */
    explicit LVWidget(LVWidget* parent = nullptr);
    
    /**
     * @brief Virtual destructor (สำคัญสำหรับ polymorphism!)
     */
    virtual ~LVWidget();
    
    // ห้าม copy (LVGL objects ไม่สามารถ copy ได้)
    LVWidget(const LVWidget&) = delete;
    LVWidget& operator=(const LVWidget&) = delete;
    
    // อนุญาต move (C++11)
    LVWidget(LVWidget&& other) noexcept;
    LVWidget& operator=(LVWidget&& other) noexcept;
    
    // ===== Property Methods (const-correct, fluent API) =====
    
    LVWidget* setSize(int32_t width, int32_t height);
    void getSize(int32_t& width, int32_t& height) const;
    
    LVWidget* setPos(int32_t x, int32_t y);
    void getPos(int32_t& x, int32_t& y) const;
    
    LVWidget* setAlign(lv_align_t align, int32_t x_offset = 0, int32_t y_offset = 0);
    
    LVWidget* show();
    LVWidget* hide();
    LVWidget* setVisible(bool visible);
    bool isVisible() const { return m_visible; }
    
    LVWidget* enable();
    LVWidget* disable();
    LVWidget* setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
    // ===== Style Methods (fluent API) =====
    
    LVWidget* setBackgroundColor(lv_color_t color);
    LVWidget* setBorderColor(lv_color_t color);
    LVWidget* setBorderWidth(int32_t width);
    LVWidget* setRadius(int32_t radius);
    
    // ===== Event Handling (modern C++ style) =====
    
    /**
     * @brief Set event handler using lambda or function
     * 
     * Example:
     *   widget->onClicked([](auto* w, auto* e) {
     *       ESP_LOGI("UI", "Clicked!");
     *   });
     */
    void onClicked(EventHandler handler) { m_onClicked = handler; }
    void onValueChanged(EventHandler handler) { m_onValueChanged = handler; }
    void onFocused(EventHandler handler) { m_onFocused = handler; }
    void onDefocused(EventHandler handler) { m_onDefocused = handler; }
    
    // ===== Virtual Event Handlers (override ในclass ลูก) =====
    
    /**
     * @brief Virtual event handlers - override เพื่อ customize behavior
     */
    virtual void handleClicked(lv_event_t* event);
    virtual void handleValueChanged(lv_event_t* event);
    virtual void handleFocused(lv_event_t* event);
    virtual void handleDefocused(lv_event_t* event);
    
    // ===== User Data =====
    
    template<typename T>
    void setUserData(T* data) { m_userData = static_cast<void*>(data); }
    
    template<typename T>
    T* getUserData() const { return static_cast<T*>(m_userData); }
    
    // ===== LVGL Object Access =====
    
    lv_obj_t* obj() const { return m_obj; }
    LVWidget* parent() const { return m_parent; }
    
protected:
    /**
     * @brief Protected constructor สำหรับ derived classes
     * @param parent Parent widget
     * @param obj LVGL object (created by derived class)
     */
    LVWidget(LVWidget* parent, lv_obj_t* obj);
    
    /**
     * @brief Initialize widget
     */
    void initialize();
    
    // LVGL object handle
    lv_obj_t* m_obj;
    
    // Parent widget
    LVWidget* m_parent;
    
private:
    // Properties
    int32_t m_x, m_y;
    int32_t m_width, m_height;
    bool m_visible;
    bool m_enabled;
    
    // Event handlers (std::function supports lambdas!)
    EventHandler m_onClicked;
    EventHandler m_onValueChanged;
    EventHandler m_onFocused;
    EventHandler m_onDefocused;
    
    // User data pointer
    void* m_userData;
    
    // Static event callback for LVGL
    static void eventCallback(lv_event_t* e);
    
    // Tag for logging
    static constexpr const char* TAG = "LVWidget";
};

#endif // LVWIDGET_HPP
