/**
 * @file LVCalendar.hpp
 * @brief C++ Wrapper สำหรับ LVGL Calendar Widget
 * 
 * Calendar = Date picker with month/year navigation
 * คล้าย DatePicker ใน Android หรือ Calendar control
 */

#ifndef LV_CALENDAR_HPP
#define LV_CALENDAR_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVCalendar
 * @brief Calendar widget สำหรับเลือกวันที่
 */
class LVCalendar : public LVWidget {
public:
    using OnDateChangeCallback = std::function<void(uint32_t year, uint32_t month, uint32_t day)>;

    explicit LVCalendar(LVWidget* parent = nullptr);

    // Date operations
    LVCalendar* setTodayDate(uint32_t year, uint32_t month, uint32_t day);
    const lv_calendar_date_t* getTodayDate() const;
    bool getPressedDate(lv_calendar_date_t* date) const;
    
    // Highlighted dates
    LVCalendar* setHighlightedDates(lv_calendar_date_t dates[], uint32_t date_num);
    
    // Navigation
    LVCalendar* setShowedDate(uint32_t year, uint32_t month);
    const lv_calendar_date_t* getShowedDate() const;
    
    // Header
    lv_obj_t* getHeaderDropdown() const;
    lv_obj_t* getHeaderArrow(lv_dir_t dir) const;
    
    // Events
    LVCalendar* onDateChange(OnDateChangeCallback callback);

private:
    OnDateChangeCallback m_onDateChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_CALENDAR_HPP
