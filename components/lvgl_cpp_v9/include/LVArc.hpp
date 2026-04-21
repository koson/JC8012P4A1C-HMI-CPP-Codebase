/**
 * @file LVArc.hpp
 * @brief C++ Wrapper สำหรับ LVGL Arc Widget
 * 
 * Arc = วงโค้งปรับค่าได้ (คล้าย knob หมุน)
 * ใช้แสดงค่าแบบวงกลม เช่น volume, brightness
 */

#ifndef LV_ARC_HPP
#define LV_ARC_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVArc
 * @brief วงโค้งปรับค่าได้
 */
class LVArc : public LVWidget {
public:
    using OnChangeCallback = std::function<void(int32_t value)>;

    explicit LVArc(LVWidget* parent = nullptr);

    /**
     * @brief กำหนดช่วงค่า
     * @param min ค่าต่ำสุด
     * @param max ค่าสูงสุด
     * @return this*
     */
    LVArc* setRange(int32_t min, int32_t max);

    /**
     * @brief กำหนดค่า
     * @param value ค่า
     * @return this*
     */
    LVArc* setValue(int32_t value);

    /**
     * @brief อ่านค่า
     */
    int32_t getValue() const;

    /**
     * @brief กำหนดมุมเริ่มต้นและมุมสิ้นสุด (องศา)
     * @param start มุมเริ่มต้น (0-360)
     * @param end มุมสิ้นสุด (0-360)
     * @return this*
     */
    LVArc* setAngles(int32_t start, int32_t end);

    /**
     * @brief กำหนดทิศทาง (หมุนตามเข็มนาฬิกาหรือทวน)
     * @param clockwise true = ตามเข็ม, false = ทวน
     * @return this*
     */
    LVArc* setRotation(int32_t rotation);

    /**
     * @brief กำหนดโหมด (normal, symmetrical, reverse)
     * @param mode โหมด
     * @return this*
     */
    LVArc* setMode(lv_arc_mode_t mode);

    /**
     * @brief Callback เมื่อค่าเปลี่ยน
     */
    LVArc* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_ARC_HPP
