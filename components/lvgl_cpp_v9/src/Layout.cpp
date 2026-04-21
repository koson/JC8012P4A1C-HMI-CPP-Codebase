#include "Layout.hpp"

// === Flex Layout ===

void Layout::flex(lv_obj_t* obj, FlexDirection direction, FlexAlign mainAlign, FlexAlign crossAlign)
{
    if (!obj) return;
    
    lv_flex_flow_t flow = getFlexFlow(direction);
    lv_obj_set_flex_flow(obj, flow);
    
    lv_flex_align_t main = getFlexAlign(mainAlign);
    lv_flex_align_t cross = getFlexAlign(crossAlign);
    lv_flex_align_t track = LV_FLEX_ALIGN_START;
    
    lv_obj_set_flex_align(obj, main, cross, track);
}

void Layout::flexRow(lv_obj_t* obj, FlexAlign mainAlign, FlexAlign crossAlign)
{
    flex(obj, FlexDirection::Row, mainAlign, crossAlign);
}

void Layout::flexColumn(lv_obj_t* obj, FlexAlign mainAlign, FlexAlign crossAlign)
{
    flex(obj, FlexDirection::Column, mainAlign, crossAlign);
}

void Layout::flexWrap(lv_obj_t* obj, bool enable)
{
    if (!obj) return;
    
    if (enable) {
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
    } else {
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    }
}

void Layout::flexGrow(lv_obj_t* obj, uint8_t grow)
{
    if (!obj) return;
    lv_obj_set_flex_grow(obj, grow);
}

// === Grid Layout ===

void Layout::grid(lv_obj_t* obj, const int32_t* cols, const int32_t* rows)
{
    if (!obj) return;
    
    lv_obj_set_grid_dsc_array(obj, cols, rows);
}

void Layout::gridEqual(lv_obj_t* obj, int numCols, int numRows)
{
    if (!obj) return;
    
    // Create column descriptors (equal size with FR units)
    static int32_t col_dsc[10];
    for (int i = 0; i < numCols && i < 9; i++) {
        col_dsc[i] = LV_GRID_FR(1);
    }
    col_dsc[numCols] = LV_GRID_TEMPLATE_LAST;
    
    // Create row descriptors
    static int32_t row_dsc[10];
    for (int i = 0; i < numRows && i < 9; i++) {
        row_dsc[i] = LV_GRID_FR(1);
    }
    row_dsc[numRows] = LV_GRID_TEMPLATE_LAST;
    
    lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc);
}

void Layout::gridCell(lv_obj_t* child, uint8_t col, uint8_t row, uint8_t colSpan, uint8_t rowSpan)
{
    if (!child) return;
    lv_obj_set_grid_cell(child, LV_GRID_ALIGN_STRETCH, col, colSpan,
                        LV_GRID_ALIGN_STRETCH, row, rowSpan);
}

// === Alignment ===

void Layout::center(lv_obj_t* obj)
{
    if (!obj) return;
    lv_obj_center(obj);
}

void Layout::align(lv_obj_t* obj, lv_align_t align, int32_t x, int32_t y)
{
    if (!obj) return;
    lv_obj_align(obj, align, x, y);
}

void Layout::alignTo(lv_obj_t* obj, lv_obj_t* ref, lv_align_t align, int32_t x, int32_t y)
{
    if (!obj || !ref) return;
    lv_obj_align_to(obj, ref, align, x, y);
}

// === Spacing & Padding ===

void Layout::setPadding(lv_obj_t* obj, int32_t padding)
{
    if (!obj) return;
    lv_obj_set_style_pad_all(obj, padding, 0);
}

void Layout::setPadding(lv_obj_t* obj, int32_t top, int32_t right, int32_t bottom, int32_t left)
{
    if (!obj) return;
    lv_obj_set_style_pad_top(obj, top, 0);
    lv_obj_set_style_pad_right(obj, right, 0);
    lv_obj_set_style_pad_bottom(obj, bottom, 0);
    lv_obj_set_style_pad_left(obj, left, 0);
}

void Layout::setGap(lv_obj_t* obj, int32_t gap)
{
    if (!obj) return;
    lv_obj_set_style_pad_column(obj, gap, 0);
    lv_obj_set_style_pad_row(obj, gap, 0);
}

void Layout::setGap(lv_obj_t* obj, int32_t columnGap, int32_t rowGap)
{
    if (!obj) return;
    lv_obj_set_style_pad_column(obj, columnGap, 0);
    lv_obj_set_style_pad_row(obj, rowGap, 0);
}

// === Size ===

void Layout::setSize(lv_obj_t* obj, int32_t width, int32_t height)
{
    if (!obj) return;
    lv_obj_set_size(obj, width, height);
}

void Layout::setWidth(lv_obj_t* obj, int32_t width)
{
    if (!obj) return;
    lv_obj_set_width(obj, width);
}

void Layout::setHeight(lv_obj_t* obj, int32_t height)
{
    if (!obj) return;
    lv_obj_set_height(obj, height);
}

void Layout::sizeFill(lv_obj_t* obj)
{
    if (!obj) return;
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
}

void Layout::sizeContent(lv_obj_t* obj)
{
    if (!obj) return;
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

// === Responsive Helpers ===

void Layout::widthPercent(lv_obj_t* obj, int32_t percent)
{
    if (!obj) return;
    lv_obj_set_width(obj, LV_PCT(percent));
}

void Layout::heightPercent(lv_obj_t* obj, int32_t percent)
{
    if (!obj) return;
    lv_obj_set_height(obj, LV_PCT(percent));
}

void Layout::responsive(lv_obj_t* obj)
{
    if (!obj) return;
    lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
}

// === Common Patterns ===

void Layout::card(lv_obj_t* obj, int32_t radius, int32_t padding)
{
    if (!obj) return;
    
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, padding, 0);
    lv_obj_set_style_shadow_width(obj, 10, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_20, 0);
    lv_obj_set_style_shadow_offset_y(obj, 5, 0);
}

void Layout::panel(lv_obj_t* obj, lv_color_t bgColor, int32_t padding)
{
    if (!obj) return;
    
    lv_obj_set_style_bg_color(obj, bgColor, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(obj, padding, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_20, 0);
}

void Layout::transparent(lv_obj_t* obj)
{
    if (!obj) return;
    
    lv_obj_set_style_bg_opa(obj, LV_OPA_0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

// === Private Helpers ===

lv_flex_flow_t Layout::getFlexFlow(FlexDirection direction)
{
    switch (direction) {
        case FlexDirection::Row:
            return LV_FLEX_FLOW_ROW;
        case FlexDirection::RowReverse:
            return LV_FLEX_FLOW_ROW_REVERSE;
        case FlexDirection::Column:
            return LV_FLEX_FLOW_COLUMN;
        case FlexDirection::ColumnReverse:
            return LV_FLEX_FLOW_COLUMN_REVERSE;
        default:
            return LV_FLEX_FLOW_ROW;
    }
}

lv_flex_align_t Layout::getFlexAlign(FlexAlign align)
{
    switch (align) {
        case FlexAlign::Start:
            return LV_FLEX_ALIGN_START;
        case FlexAlign::Center:
            return LV_FLEX_ALIGN_CENTER;
        case FlexAlign::End:
            return LV_FLEX_ALIGN_END;
        case FlexAlign::SpaceBetween:
            return LV_FLEX_ALIGN_SPACE_BETWEEN;
        case FlexAlign::SpaceAround:
            return LV_FLEX_ALIGN_SPACE_AROUND;
        case FlexAlign::SpaceEvenly:
            return LV_FLEX_ALIGN_SPACE_EVENLY;
        default:
            return LV_FLEX_ALIGN_START;
    }
}
