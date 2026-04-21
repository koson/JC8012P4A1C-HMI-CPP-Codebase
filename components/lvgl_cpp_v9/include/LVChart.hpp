/**
 * @file LVChart.hpp
 * @brief C++ Wrapper สำหรับ LVGL Chart Widget
 * 
 * Chart = Line/Bar/Scatter chart for data visualization
 * คล้าย QChart ใน Qt หรือ Chart.js
 */

#ifndef LV_CHART_HPP
#define LV_CHART_HPP

#include "LVWidget.hpp"
#include <functional>
#include <vector>

/**
 * @class LVChart
 * @brief Chart widget สำหรับแสดงกราฟ
 */
class LVChart : public LVWidget {
public:
    explicit LVChart(LVWidget* parent = nullptr);

    // Chart configuration
    LVChart* setType(lv_chart_type_t type);
    LVChart* setPointCount(uint32_t count);
    LVChart* setRange(lv_chart_axis_t axis, int32_t min, int32_t max);
    LVChart* setDivLineCount(uint8_t hdiv, uint8_t vdiv);
    
    // Series management
    lv_chart_series_t* addSeries(lv_color_t color, lv_chart_axis_t axis);
    LVChart* removeSeries(lv_chart_series_t* series);
    LVChart* hideSeriesAll(bool hide);
    
    // Data operations
    LVChart* setSeriesValue(lv_chart_series_t* series, uint32_t index, int32_t value);
    LVChart* setAllSeriesValue(lv_chart_series_t* series, int32_t value);
    LVChart* setNextSeriesValue(lv_chart_series_t* series, int32_t value);
    LVChart* setSeriesPoints(lv_chart_series_t* series, const std::vector<int32_t>& values);
    
    // Cursor
    lv_chart_cursor_t* addCursor(lv_color_t color, lv_dir_t dir);
    LVChart* setCursorPoint(lv_chart_cursor_t* cursor, lv_chart_series_t* series, uint32_t index);
    
    // Update
    LVChart* refresh();

private:
    uint32_t m_pointCount = 10;
};

#endif // LV_CHART_HPP
