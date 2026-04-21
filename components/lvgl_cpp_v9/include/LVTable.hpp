/**
 * @file LVTable.hpp
 * @brief C++ Wrapper สำหรับ LVGL Table Widget
 * 
 * Table = Data table with cells, rows, columns
 * คล้าย QTableWidget ใน Qt หรือ DataGrid
 */

#ifndef LV_TABLE_HPP
#define LV_TABLE_HPP

#include "LVWidget.hpp"
#include <string>
#include <vector>

/**
 * @class LVTable
 * @brief Table widget สำหรับแสดงตารางข้อมูล
 */
class LVTable : public LVWidget {
public:
    explicit LVTable(LVWidget* parent = nullptr);

    // Table structure
    LVTable* setRowCount(uint32_t count);
    LVTable* setColumnCount(uint32_t count);
    uint32_t getRowCount() const;
    uint32_t getColumnCount() const;
    
    // Cell operations
    LVTable* setCellValue(uint32_t row, uint32_t col, const char* text);
    const char* getCellValue(uint32_t row, uint32_t col) const;
    LVTable* setCellValueFormat(uint32_t row, uint32_t col, const char* fmt, ...);
    
    // Column width
    LVTable* setColumnWidth(uint32_t col, int32_t width);
    
    // Cell control
    LVTable* setCellControl(uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl);
    LVTable* clearCellControl(uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl);
    bool hasCellControl(uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl) const;
    
    // Selected cell
    LVTable* getSelectedCell(uint32_t* row, uint32_t* col) const;

private:
    uint32_t m_rows = 1;
    uint32_t m_cols = 1;
};

#endif // LV_TABLE_HPP
