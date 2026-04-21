/**
 * @file LVCheckbox.hpp
 * @brief C++ Wrapper สำหรับ LVGL Checkbox Widget
 * 
 * Checkbox = ช่องติ๊กที่มีข้อความ (คล้าย <input type="checkbox"> ใน HTML)
 * คล้าย CButton(BS_CHECKBOX) ใน MFC
 */

#ifndef LV_CHECKBOX_HPP
#define LV_CHECKBOX_HPP

#include "LVWidget.hpp"
#include <functional>
#include <string>

/**
 * @class LVCheckbox
 * @brief ช่องติ๊กพร้อมข้อความ
 */
class LVCheckbox : public LVWidget {
public:
    /**
     * @brief Callback เมื่อสถานะเปลี่ยน
     * @param checked true = ติ๊ก, false = ไม่ติ๊ก
     */
    using OnChangeCallback = std::function<void(bool checked)>;

    /**
     * @brief สร้าง Checkbox
     * @param parent Widget แม่
     * @param text ข้อความที่แสดง
     */
    explicit LVCheckbox(LVWidget* parent = nullptr, const std::string& text = "");

    /**
     * @brief กำหนดข้อความ
     * @param text ข้อความใหม่
     * @return this*
     */
    LVCheckbox* setText(const std::string& text);

    /**
     * @brief ติ๊กหรือเอาออก
     * @param checked true = ติ๊ก, false = เอาออก
     * @return this*
     */
    LVCheckbox* setChecked(bool checked);

    /**
     * @brief เช็คว่าติ๊กอยู่หรือไม่
     * @return true = ติ๊กอยู่, false = ไม่ติ๊ก
     */
    bool isChecked() const;

    /**
     * @brief Toggle สถานะ (ติ๊ก <-> ไม่ติ๊ก)
     * @return this*
     */
    LVCheckbox* toggle();

    /**
     * @brief กำหนด callback เมื่อสถานะเปลี่ยน
     * @param callback ฟังก์ชันที่จะเรียก
     * @return this*
     */
    LVCheckbox* onChange(OnChangeCallback callback);

private:
    OnChangeCallback m_onChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_CHECKBOX_HPP
