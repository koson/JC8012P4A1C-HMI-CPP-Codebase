/**
 * @file LVChart.cpp
 * @brief Implementation ของ LVChart
 */

#include "../include/LVChart.hpp"

LVChart::LVChart(LVWidget* parent)
    : LVWidget(parent, lv_chart_create(parent ? parent->obj() : lv_scr_act()))
{
    if (!m_obj) return;
}

LVChart* LVChart::setType(lv_chart_type_t type)
{
    lv_chart_set_type(m_obj, type);
    return this;
}

LVChart* LVChart::setPointCount(uint32_t count)
{
    m_pointCount = count;
    lv_chart_set_point_count(m_obj, count);
    return this;
}

LVChart* LVChart::setRange(lv_chart_axis_t axis, int32_t min, int32_t max)
{
    lv_chart_set_range(m_obj, axis, min, max);
    return this;
}

LVChart* LVChart::setDivLineCount(uint8_t hdiv, uint8_t vdiv)
{
    lv_chart_set_div_line_count(m_obj, hdiv, vdiv);
    return this;
}

lv_chart_series_t* LVChart::addSeries(lv_color_t color, lv_chart_axis_t axis)
{
    return lv_chart_add_series(m_obj, color, axis);
}

LVChart* LVChart::removeSeries(lv_chart_series_t* series)
{
    lv_chart_remove_series(m_obj, series);
    return this;
}

LVChart* LVChart::hideSeriesAll(bool hide)
{
    lv_chart_hide_series(m_obj, nullptr, hide);
    return this;
}

LVChart* LVChart::setSeriesValue(lv_chart_series_t* series, uint32_t index, int32_t value)
{
    lv_chart_set_value_by_id(m_obj, series, index, value);
    return this;
}

LVChart* LVChart::setAllSeriesValue(lv_chart_series_t* series, int32_t value)
{
    lv_chart_set_all_value(m_obj, series, value);
    return this;
}

LVChart* LVChart::setNextSeriesValue(lv_chart_series_t* series, int32_t value)
{
    lv_chart_set_next_value(m_obj, series, value);
    return this;
}

LVChart* LVChart::setSeriesPoints(lv_chart_series_t* series, const std::vector<int32_t>& values)
{
    for (size_t i = 0; i < values.size() && i < m_pointCount; i++) {
        lv_chart_set_value_by_id(m_obj, series, i, values[i]);
    }
    return this;
}

lv_chart_cursor_t* LVChart::addCursor(lv_color_t color, lv_dir_t dir)
{
    return lv_chart_add_cursor(m_obj, color, dir);
}

LVChart* LVChart::setCursorPoint(lv_chart_cursor_t* cursor, lv_chart_series_t* series, uint32_t index)
{
    lv_chart_set_cursor_point(m_obj, cursor, series, index);
    return this;
}

LVChart* LVChart::refresh()
{
    lv_chart_refresh(m_obj);
    return this;
}
