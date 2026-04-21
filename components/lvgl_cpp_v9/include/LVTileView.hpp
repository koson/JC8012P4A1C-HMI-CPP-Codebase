/**
 * @file LVTileView.hpp
 * @brief C++ Wrapper สำหรับ LVGL TileView Widget
 * 
 * TileView = Container for sliding between pages/tiles
 * คล้าย ViewPager ใน Android
 */

#ifndef LV_TILEVIEW_HPP
#define LV_TILEVIEW_HPP

#include "LVWidget.hpp"
#include <functional>

/**
 * @class LVTileView
 * @brief TileView widget สำหรับเลื่อนระหว่างหน้า
 */
class LVTileView : public LVWidget {
public:
    using OnTileChangeCallback = std::function<void(lv_obj_t* tile)>;

    explicit LVTileView(LVWidget* parent = nullptr);

    // Tile management
    lv_obj_t* addTile(uint8_t col, uint8_t row, lv_dir_t dir);
    
    // Navigation
    LVTileView* setTile(lv_obj_t* tile, lv_anim_enable_t anim = LV_ANIM_OFF);
    LVTileView* setTileByIndex(uint8_t col, uint8_t row, lv_anim_enable_t anim = LV_ANIM_OFF);
    
    // Events
    LVTileView* onTileChange(OnTileChangeCallback callback);

private:
    OnTileChangeCallback m_onTileChange;
    static void eventHandler(lv_event_t* e);
};

#endif // LV_TILEVIEW_HPP
