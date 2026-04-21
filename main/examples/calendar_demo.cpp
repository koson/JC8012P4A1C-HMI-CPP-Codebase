/**
 * @file calendar_demo.cpp
 * @brief Demo for LVCalendar - Date picker widget testing
 * 
 * Tests:
 * - Calendar with month/year navigation
 * - Date selection
 * - Highlighted dates
 * - Date change callbacks
 */

#include "../include/LVPanel.hpp"
#include "../include/LVCalendar.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include <ctime>

class CalendarDemoScreen : public LVPanel {
public:
    CalendarDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0xF5F5F5), LV_PART_MAIN);
        createUI();
    }

private:
    LVCalendar* calendar = nullptr;
    LVLabel* selectedDateLabel = nullptr;
    LVLabel* infoLabel = nullptr;
    
    void createUI() {
        // Title
        auto* title = new LVLabel(this);
        title->setText("Calendar Demo");
        title->setTextColor(lv_color_hex(0x333333));
        title->setAlign(LV_ALIGN_TOP_MID);
        lv_obj_set_y(title->obj(), 10);
        lv_obj_set_style_text_font(title->obj(), &lv_font_montserrat_24, 0);
        
        // Calendar widget
        calendar = new LVCalendar(this);
        calendar->setAlign(LV_ALIGN_TOP_LEFT);
        lv_obj_set_pos(calendar->obj(), 30, 60);
        
        // Create header with navigation arrows
        lv_calendar_header_arrow_create(calendar->obj());
        
        // Set today's date
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        uint32_t year = timeinfo->tm_year + 1900;
        uint32_t month = timeinfo->tm_mon + 1;
        uint32_t day = timeinfo->tm_mday;
        
        calendar->setTodayDate(year, month, day);
        calendar->setShowedDate(year, month);
        
        // Highlight some special dates
        lv_calendar_date_t highlighted[] = {
            {(uint16_t)year, (uint8_t)month, 1},   // First day of month
            {(uint16_t)year, (uint8_t)month, 15},  // Mid month
            {(uint16_t)year, (uint8_t)month, (uint8_t)day}  // Today
        };
        calendar->setHighlightedDates(highlighted, 3);
        
        // Date change callback
        calendar->onDateChange([this](uint32_t year, uint32_t month, uint32_t day) {
            char buf[64];
            const char* monthNames[] = {
                "January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October", "November", "December"
            };
            sprintf(buf, "Selected: %s %d, %d", monthNames[month - 1], (int)day, (int)year);
            selectedDateLabel->setText(buf);
        });
        
        // Selected date label
        selectedDateLabel = new LVLabel(this);
        selectedDateLabel->setText("Selected: (Click a date)");
        selectedDateLabel->setTextColor(lv_color_hex(0x0066CC));
        lv_obj_set_pos(selectedDateLabel->obj(), 480, 80);
        lv_obj_set_style_text_font(selectedDateLabel->obj(), &lv_font_montserrat_18, 0);
        
        // Info panel
        auto* infoPanel = new LVPanel(this);
        infoPanel->setSize(350, 280);
        lv_obj_set_pos(infoPanel->obj(), 480, 130);
        lv_obj_set_style_bg_color(infoPanel->obj(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(infoPanel->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(infoPanel->obj(), lv_color_hex(0xDDDDDD), LV_PART_MAIN);
        lv_obj_set_style_pad_all(infoPanel->obj(), 20, LV_PART_MAIN);
        
        // Info title
        auto* infoTitle = new LVLabel(infoPanel);
        infoTitle->setText("Calendar Features");
        infoTitle->setTextColor(lv_color_hex(0x333333));
        lv_obj_set_pos(infoTitle->obj(), 0, 0);
        lv_obj_set_style_text_font(infoTitle->obj(), &lv_font_montserrat_18, 0);
        
        // Info text
        infoLabel = new LVLabel(infoPanel);
        infoLabel->setText(
            "• Click date to select\n"
            "• Use arrows to navigate\n"
            "• Dropdown for month/year\n"
            "• Highlighted dates shown\n\n"
            "Current month displayed\n"
            "with navigation controls"
        );
        infoLabel->setTextColor(lv_color_hex(0x666666));
        lv_obj_set_pos(infoLabel->obj(), 0, 35);
        lv_obj_set_style_text_line_space(infoLabel->obj(), 8, 0);
        
        // Today button
        auto* todayBtn = new LVButton(infoPanel);
        todayBtn->setSize(150, 45);
        lv_obj_set_pos(todayBtn->obj(), 0, 200);
        todayBtn->setText("Go to Today");
        lv_obj_set_style_bg_color(todayBtn->obj(), lv_color_hex(0x2196F3), LV_PART_MAIN);
        
        todayBtn->onClicked([this](LVWidget* w, lv_event_t* e) {
            time_t now = time(nullptr);
            struct tm* timeinfo = localtime(&now);
            uint32_t year = timeinfo->tm_year + 1900;
            uint32_t month = timeinfo->tm_mon + 1;
            
            calendar->setShowedDate(year, month);
            selectedDateLabel->setText("Showing current month");
        });
    }
};

extern "C" void create_calendar_demo() {
    auto* screen = new CalendarDemoScreen();
    lv_obj_set_parent(screen->obj(), lv_screen_active());
}
