/**
 * @file LVTileView.cpp
 * @brief Implementation ของ LVTileView
 */

#include "../include/LVTileView.hpp"

LVTileView::LVTileView(LVWidget* parent)
    : LVWidget(parent, lv_tileview_create(parent ? parent->obj() : nullptr))
{
}

lv_obj_t* LVTileView::addTile(uint8_t col, uint8_t row, lv_dir_t dir)
{
    return lv_tileview_add_tile(m_obj, col, row, dir);
}

LVTileView* LVTileView::setTile(lv_obj_t* tile, lv_anim_enable_t anim)
{
    lv_tileview_set_tile(m_obj, tile, anim);
    return this;
}

LVTileView* LVTileView::setTileByIndex(uint8_t col, uint8_t row, lv_anim_enable_t anim)
{
    lv_tileview_set_tile_by_index(m_obj, col, row, anim);
    return this;
}

LVTileView* LVTileView::onTileChange(OnTileChangeCallback callback)
{
    m_onTileChange = callback;
    lv_obj_add_event_cb(m_obj, eventHandler, LV_EVENT_VALUE_CHANGED, nullptr);
    return this;
}

void LVTileView::eventHandler(lv_event_t* e)
{
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    LVTileView* tileview = static_cast<LVTileView*>(lv_obj_get_user_data(obj));
    
    if (tileview && tileview->m_onTileChange) {
        lv_obj_t* tile = lv_tileview_get_tile_active(obj);
        tileview->m_onTileChange(tile);
    }
}
