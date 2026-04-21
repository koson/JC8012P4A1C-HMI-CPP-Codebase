/**
 * @file LVCalendar.cpp
 * @brief Implementation ของ LVCalendar
 */

#include "../include/LVCalendar.hpp"
#include <ctime>

LVCalendar::LVCalendar(LVWidget* parent)
    : LVWidget(parent, lv_calendar_create(parent ? parent->obj() : nullptr))
{
}

LVCalendar* LVCalendar::setTodayDate(uint32_t year, uint32_t month, uint32_t day)
{
    lv_calendar_set_today_date(m_obj, year, month, day);
    return this;
}

const lv_calendar_date_t* LVCalendar::getTodayDate() const
{
    return lv_calendar_get_today_date(m_obj);
}

bool LVCalendar::getPressedDate(lv_calendar_date_t* date) const
{
    return lv_calendar_get_pressed_date(m_obj, date) == LV_RESULT_OK;
}

LVCalendar* LVCalendar::setHighlightedDates(lv_calendar_date_t dates[], uint32_t date_num)
{
    lv_calendar_set_highlighted_dates(m_obj, dates, date_num);
    return this;
}

LVCalendar* LVCalendar::setShowedDate(uint32_t year, uint32_t month)
{
    lv_calendar_set_showed_date(m_obj, year, month);
    return this;
}

const lv_calendar_date_t* LVCalendar::getShowedDate() const
{
    return lv_calendar_get_showed_date(m_obj);
}

lv_obj_t* LVCalendar::getHeaderDropdown() const
{
    return lv_calendar_header_dropdown_create(m_obj);
}

lv_obj_t* LVCalendar::getHeaderArrow(lv_dir_t dir) const
{
    return lv_calendar_header_arrow_create(m_obj);
}

LVCalendar* LVCalendar::onDateChange(OnDateChangeCallback callback)
{
    m_onDateChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVCalendar::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVCalendar* calendar = static_cast<LVCalendar*>(lv_obj_get_user_data(obj));
    
    if (calendar && calendar->m_onDateChange) {
        lv_calendar_date_t date;
        if (lv_calendar_get_pressed_date(obj, &date) == LV_RESULT_OK) {
            calendar->m_onDateChange(date.year, date.month, date.day);
        }
    }
}
