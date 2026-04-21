/**
 * @file LVAnimImg.hpp
 * @brief C++ Wrapper สำหรับ LVGL AnimImg Widget
 * 
 * AnimImg = Animated image player
 * คล้าย GIF player หรือ Sprite animator
 */

#ifndef LV_ANIMIMG_HPP
#define LV_ANIMIMG_HPP

#include "LVWidget.hpp"

/**
 * @class LVAnimImg
 * @brief Animated image widget
 */
class LVAnimImg : public LVWidget {
public:
    explicit LVAnimImg(LVWidget* parent = nullptr);

    // Image sources
    LVAnimImg* setImages(const void** images, uint8_t count);
    
    // Animation control
    LVAnimImg* setDuration(uint32_t duration);
    LVAnimImg* setRepeatCount(uint16_t count);
    LVAnimImg* start();
    
    // Source index
    LVAnimImg* setSourceIndex(uint8_t index);
    uint8_t getSourceIndex() const;

private:
    const void** m_images = nullptr;
    uint8_t m_count = 0;
};

#endif // LV_ANIMIMG_HPP
