/**
 * @file LVSlider.hpp
 * @brief C++ Wrapper สำหรับ LVGL Slider Widget
 * 
 * PATTERN EXPLANATION (รูปแบบการสร้าง Widget):
 * ==========================================
 * 1. สืบทอดจาก LVWidget - ได้ความสามารถพื้นฐานทั้งหมด
 * 2. Constructor รับ parent และสร้าง lv_slider_create()
 * 3. ห่อหุ้ม LVGL functions เป็น methods ที่ใช้งานง่าย
 * 4. Return this* เพื่อทำ method chaining
 * 5. ใช้ std::function สำหรับ event callbacks
 * 
 * USAGE EXAMPLE:
 * ==============
 * auto slider = new LVSlider(parent);
 * slider->setRange(0, 100)
 *       ->setValue(50)
 *       ->setSize(200, 20)
 *       ->onChange([](int value) {
 *           printf("Value: %d\n", value);
 *       });
 */

#ifndef LV_SLIDER_HPP
#define LV_SLIDER_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVSlider
 * @brief แถบเลื่อนสำหรับปรับค่า (Slider Widget)
 * 
 * คล้ายกับ CSliderCtrl ใน MFC หรือ <input type="range"> ใน HTML
 */
class LVSlider : public LVWidget {
public:
    /**
     * @brief Callback สำหรับเมื่อค่าเปลี่ยน
     * @param value ค่าปัจจุบันของ slider (int32_t)
     */
    using OnChangeCallback = std::function<void(int32_t value)>;

    /**
     * @brief สร้าง Slider Widget
     * @param parent Widget แม่ (nullptr = screen)
     * 
     * PATTERN: Constructor จะ:
     * 1. เรียก parent constructor (LVWidget)
     * 2. สร้าง LVGL object ด้วย lv_slider_create()
     * 3. ตั้งค่าเริ่มต้นตามต้องการ
     */
    explicit LVSlider(LVWidget* parent = nullptr);

    /**
     * @brief กำหนดช่วงค่า (min, max)
     * @param min ค่าต่ำสุด
     * @param max ค่าสูงสุด
     * @return this* สำหรับ method chaining
     * 
     * PATTERN: ห่อหุ้ม lv_slider_set_range()
     * Return this* เพื่อให้ chain ได้: setRange(0,100)->setValue(50)
     */
    LVSlider* setRange(int32_t min, int32_t max);

    /**
     * @brief กำหนดค่าปัจจุบัน
     * @param value ค่าที่ต้องการตั้ง
     * @param anim true = มี animation, false = เปลี่ยนทันที
     * @return this* สำหรับ method chaining
     */
    LVSlider* setValue(int32_t value, bool anim = false);

    /**
     * @brief อ่านค่าปัจจุบัน
     * @return ค่าปัจจุบันของ slider
     */
    int32_t getValue() const;

    /**
     * @brief กำหนดค่าซ้าย (สำหรับ range slider)
     * @param value ค่าซ้าย
     * @param anim มี animation หรือไม่
     * @return this*
     */
    LVSlider* setLeftValue(int32_t value, bool anim = false);

    /**
     * @brief อ่านค่าซ้าย
     * @return ค่าซ้ายของ slider
     */
    int32_t getLeftValue() const;

    /**
     * @brief เปิดใช้งานโหมด range (มี 2 knobs)
     * @param enable true = range mode, false = normal mode
     * @return this*
     */
    LVSlider* setMode(bool isRange);

    /**
     * @brief กำหนด callback เมื่อค่าเปลี่ยน
     * @param callback ฟังก์ชันที่จะเรียกเมื่อค่าเปลี่ยน
     * @return this*
     * 
     * PATTERN: ใช้ std::function รับได้ทั้ง lambda, function pointer, std::bind
     * Example: slider->onChange([](int val) { printf("%d\n", val); });
     */
    LVSlider* onChange(OnChangeCallback callback);

    // เพิ่มความสามารถเฉพาะ slider
    
    /**
     * @brief ทำให้ slider อยู่ในโหมดสมมาตร (symmetric)
     * ค่ากลางจะอยู่ที่ 0
     */
    LVSlider* setSymmetric(bool enable);

private:
    OnChangeCallback m_onChange;  ///< เก็บ callback สำหรับ onChange event
    
    /**
     * @brief Event handler ภายใน
     * จะถูกเรียกจาก LVGL event system และแปลงเป็น C++ callback
     */
    static void eventHandler(lv_event_t* e);
};

#endif // LV_SLIDER_HPP
