/**
 * @file LVTable.cpp
 * @brief Implementation ของ LVTable
 */

#include "../include/LVTable.hpp"
#include <cstdarg>
#include <cstdio>

LVTable::LVTable(LVWidget* parent)
    : LVWidget(parent, lv_table_create(parent ? parent->obj() : nullptr))
{
}

LVTable* LVTable::setRowCount(uint32_t count)
{
    m_rows = count;
    lv_table_set_row_count(m_obj, count);
    return this;
}

LVTable* LVTable::setColumnCount(uint32_t count)
{
    m_cols = count;
    lv_table_set_column_count(m_obj, count);
    return this;
}

uint32_t LVTable::getRowCount() const
{
    return lv_table_get_row_count(m_obj);
}

uint32_t LVTable::getColumnCount() const
{
    return lv_table_get_column_count(m_obj);
}

LVTable* LVTable::setCellValue(uint32_t row, uint32_t col, const char* text)
{
    lv_table_set_cell_value(m_obj, row, col, text);
    return this;
}

const char* LVTable::getCellValue(uint32_t row, uint32_t col) const
{
    return lv_table_get_cell_value(m_obj, row, col);
}

LVTable* LVTable::setCellValueFormat(uint32_t row, uint32_t col, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    lv_table_set_cell_value(m_obj, row, col, buf);
    return this;
}

LVTable* LVTable::setColumnWidth(uint32_t col, int32_t width)
{
    lv_table_set_column_width(m_obj, col, width);
    return this;
}

LVTable* LVTable::setCellControl(uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl)
{
    lv_table_set_cell_ctrl(m_obj, row, col, ctrl);
    return this;
}

LVTable* LVTable::clearCellControl(uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl)
{
    lv_table_clear_cell_ctrl(m_obj, row, col, ctrl);
    return this;
}

bool LVTable::hasCellControl(uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl) const
{
    return lv_table_has_cell_ctrl(m_obj, row, col, ctrl);
}

LVTable* LVTable::getSelectedCell(uint32_t* row, uint32_t* col) const
{
    lv_table_get_selected_cell(m_obj, row, col);
    return const_cast<LVTable*>(this);
}
