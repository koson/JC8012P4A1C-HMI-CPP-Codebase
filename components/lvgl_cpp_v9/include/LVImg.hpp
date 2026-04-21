/**
 * @file LVImg.hpp
 * @brief C++ Wrapper สำหรับ LVGL Image Widget
 * 
 * LVImg = Image display widget
 * แสดงรูปภาพจาก file, memory buffer, symbol
 */

#ifndef LV_IMG_HPP
#define LV_IMG_HPP

#include "LVWidget.hpp"
#include <string>

/**
 * @class LVImg
 * @brief Image display widget
 */
class LVImg : public LVWidget {
public:
    explicit LVImg(LVWidget* parent = nullptr);

    // Image source
    LVImg* setSource(const void* src);
    LVImg* setSource(const std::string& path);
    LVImg* setSymbol(const char* symbol);

    // Transformation
    LVImg* setAngle(int16_t angle);  // 0-3600 (0.1 degree steps)
    LVImg* setZoom(uint16_t zoom);   // 256 = 100%
    LVImg* setPivot(int16_t x, int16_t y);

    // Offset
    LVImg* setOffset(int16_t x, int16_t y);

    // Antialias
    LVImg* setAntialias(bool enable);
};

#endif // LV_IMG_HPP
