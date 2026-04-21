#pragma once

#include "lvgl.h"

/**
 * @brief Layout helper utilities for LVGL
 * 
 * Provides static helper functions for common layout patterns:
 * - Flex layouts (row, column, wrap)
 * - Grid layouts
 * - Alignment helpers
 * - Spacing and padding
 * - Responsive utilities
 * 
 * Usage:
 * @code
 * lv_obj_t* container = lv_obj_create(parent);
 * Layout::flex(container, Layout::FlexDirection::Column, Layout::FlexAlign::Center);
 * Layout::setPadding(container, 10);
 * Layout::setGap(container, 5);
 * @endcode
 */
class Layout {
public:
    // Flex direction
    enum class FlexDirection {
        Row,            // Left to right
        RowReverse,     // Right to left
        Column,         // Top to bottom
        ColumnReverse   // Bottom to top
    };
    
    // Flex alignment
    enum class FlexAlign {
        Start,          // Align to start
        Center,         // Center alignment
        End,            // Align to end
        SpaceBetween,   // Space between items
        SpaceAround,    // Space around items
        SpaceEvenly     // Even spacing
    };
    
    // === Flex Layout ===
    
    /**
     * @brief Setup flex container
     * @param obj Container object
     * @param direction Flex direction
     * @param mainAlign Main axis alignment
     * @param crossAlign Cross axis alignment
     */
    static void flex(lv_obj_t* obj, 
                    FlexDirection direction = FlexDirection::Row,
                    FlexAlign mainAlign = FlexAlign::Start,
                    FlexAlign crossAlign = FlexAlign::Start);
    
    /**
     * @brief Setup flex row container
     */
    static void flexRow(lv_obj_t* obj, 
                       FlexAlign mainAlign = FlexAlign::Start,
                       FlexAlign crossAlign = FlexAlign::Start);
    
    /**
     * @brief Setup flex column container
     */
    static void flexColumn(lv_obj_t* obj,
                          FlexAlign mainAlign = FlexAlign::Start,
                          FlexAlign crossAlign = FlexAlign::Start);
    
    /**
     * @brief Enable/disable flex wrap
     */
    static void flexWrap(lv_obj_t* obj, bool enable = true);
    
    /**
     * @brief Set flex grow factor for child
     */
    static void flexGrow(lv_obj_t* obj, uint8_t grow);

    // === Grid Layout ===
    
    /**
     * @brief Setup grid container
     * @param obj Container object
     * @param cols Column descriptors (e.g., {100, 100, LV_GRID_TEMPLATE_LAST})
     * @param rows Row descriptors
     */
    static void grid(lv_obj_t* obj,
                    const int32_t* cols,
                    const int32_t* rows);
    
    /**
     * @brief Create equal columns grid
     * @param obj Container object
     * @param numCols Number of columns
     * @param numRows Number of rows
     */
    static void gridEqual(lv_obj_t* obj, int numCols, int numRows);
    
    /**
     * @brief Place child in grid cell
     */
    static void gridCell(lv_obj_t* child,
                        uint8_t col, uint8_t row,
                        uint8_t colSpan = 1, uint8_t rowSpan = 1);

    // === Alignment ===
    
    /**
     * @brief Center object in parent
     */
    static void center(lv_obj_t* obj);
    
    /**
     * @brief Align object
     */
    static void align(lv_obj_t* obj, lv_align_t align, int32_t x = 0, int32_t y = 0);
    
    /**
     * @brief Align to another object
     */
    static void alignTo(lv_obj_t* obj, lv_obj_t* ref, lv_align_t align, int32_t x = 0, int32_t y = 0);

    // === Spacing & Padding ===
    
    /**
     * @brief Set padding (all sides)
     */
    static void setPadding(lv_obj_t* obj, int32_t padding);
    
    /**
     * @brief Set padding (individual sides)
     */
    static void setPadding(lv_obj_t* obj, int32_t top, int32_t right, int32_t bottom, int32_t left);
    
    /**
     * @brief Set gap between flex/grid items
     */
    static void setGap(lv_obj_t* obj, int32_t gap);
    
    /**
     * @brief Set column and row gap
     */
    static void setGap(lv_obj_t* obj, int32_t columnGap, int32_t rowGap);

    // === Size ===
    
    /**
     * @brief Set size
     */
    static void setSize(lv_obj_t* obj, int32_t width, int32_t height);
    
    /**
     * @brief Set width
     */
    static void setWidth(lv_obj_t* obj, int32_t width);
    
    /**
     * @brief Set height
     */
    static void setHeight(lv_obj_t* obj, int32_t height);
    
    /**
     * @brief Set size to fill parent
     */
    static void sizeFill(lv_obj_t* obj);
    
    /**
     * @brief Set size to content
     */
    static void sizeContent(lv_obj_t* obj);

    // === Responsive Helpers ===
    
    /**
     * @brief Set width as percentage
     */
    static void widthPercent(lv_obj_t* obj, int32_t percent);
    
    /**
     * @brief Set height as percentage
     */
    static void heightPercent(lv_obj_t* obj, int32_t percent);
    
    /**
     * @brief Make object responsive (fill width, content height)
     */
    static void responsive(lv_obj_t* obj);

    // === Common Patterns ===
    
    /**
     * @brief Create card container (rounded, shadow, padding)
     */
    static void card(lv_obj_t* obj, int32_t radius = 10, int32_t padding = 15);
    
    /**
     * @brief Create panel (background, border, padding)
     */
    static void panel(lv_obj_t* obj, lv_color_t bgColor, int32_t padding = 10);
    
    /**
     * @brief Remove all styling (transparent background, no border)
     */
    static void transparent(lv_obj_t* obj);

private:
    // Helper to convert enum to LVGL constants
    static lv_flex_flow_t getFlexFlow(FlexDirection direction);
    static lv_flex_align_t getFlexAlign(FlexAlign align);
};
