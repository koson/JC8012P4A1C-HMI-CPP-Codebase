/**
 * @file LVImg.cpp
 * @brief Implementation ของ LVImg
 */

#include "../include/LVImg.hpp"

LVImg::LVImg(LVWidget* parent)
    : LVWidget(parent, lv_image_create(parent ? parent->obj() : nullptr))
{
}

LVImg* LVImg::setSource(const void* src)
{
    lv_image_set_src(m_obj, src);
    return this;
}

LVImg* LVImg::setSource(const std::string& path)
{
    lv_image_set_src(m_obj, path.c_str());
    return this;
}

LVImg* LVImg::setSymbol(const char* symbol)
{
    lv_image_set_src(m_obj, symbol);
    return this;
}

LVImg* LVImg::setAngle(int16_t angle)
{
    lv_image_set_rotation(m_obj, angle);
    return this;
}

LVImg* LVImg::setZoom(uint16_t zoom)
{
    lv_image_set_scale(m_obj, zoom);
    return this;
}

LVImg* LVImg::setPivot(int16_t x, int16_t y)
{
    lv_image_set_pivot(m_obj, x, y);
    return this;
}

LVImg* LVImg::setOffset(int16_t x, int16_t y)
{
    lv_image_set_offset_x(m_obj, x);
    lv_image_set_offset_y(m_obj, y);
    return this;
}

LVImg* LVImg::setAntialias(bool enable)
{
    lv_obj_set_style_image_recolor_opa(m_obj, enable ? LV_OPA_TRANSP : LV_OPA_COVER, LV_PART_MAIN);
    return this;
}
