/**
 * @file LVAnimImg.cpp
 * @brief Implementation ของ LVAnimImg
 */

#include "../include/LVAnimImg.hpp"

LVAnimImg::LVAnimImg(LVWidget* parent)
    : LVWidget(parent, lv_animimg_create(parent ? parent->obj() : nullptr))
{
}

LVAnimImg* LVAnimImg::setImages(const void** images, uint8_t count)
{
    m_images = images;
    m_count = count;
    lv_animimg_set_src(m_obj, images, count);
    return this;
}

LVAnimImg* LVAnimImg::setDuration(uint32_t duration)
{
    lv_animimg_set_duration(m_obj, duration);
    return this;
}

LVAnimImg* LVAnimImg::setRepeatCount(uint16_t count)
{
    lv_animimg_set_repeat_count(m_obj, count);
    return this;
}

LVAnimImg* LVAnimImg::start()
{
    lv_animimg_start(m_obj);
    return this;
}

LVAnimImg* LVAnimImg::setSourceIndex(uint8_t index)
{
    if (m_images && index < m_count) {
        lv_image_set_src(m_obj, m_images[index]);
    }
    return this;
}

uint8_t LVAnimImg::getSourceIndex() const
{
    // LVGL doesn't have direct API for this, return 0
    return 0;
}
