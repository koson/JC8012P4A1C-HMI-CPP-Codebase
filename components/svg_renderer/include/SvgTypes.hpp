#pragma once

#include "DrawTarget.hpp"
#include <vector>
#include <cstdint>

namespace SvgRenderer {

/**
 * @brief 2D Point structure
 */
struct Point {
    float x;
    float y;
    
    Point() : x(0), y(0) {}
    Point(float _x, float _y) : x(_x), y(_y) {}
};

/**
 * @brief ViewBox for SVG symbols
 */
struct ViewBox {
    float x;
    float y;
    float width;
    float height;
    
    ViewBox() : x(0), y(0), width(0), height(0) {}
    ViewBox(float _x, float _y, float _w, float _h) 
        : x(_x), y(_y), width(_w), height(_h) {}
};

/**
 * @brief SVG Path Command (alias to global VectorPathCommand)
 */
using PathCommand = VectorPathCommand;

/**
 * @brief SVG Symbol Definition
 */
struct SvgSymbol {
    const char* id;             // Symbol ID (e.g., "Xor_Gate")
    const char* pathData;       // SVG path string
    ViewBox viewBox;            // ViewBox coordinates
    float scale;                // Default scale
    float rotation;             // Default rotation (degrees)
    
    SvgSymbol() 
        : id(nullptr)
        , pathData(nullptr)
        , viewBox()
        , scale(1.0f)
        , rotation(0.0f) {}
};

/**
 * @brief Color structure (RGB)
 */
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    
    Color() : r(0), g(0), b(0) {}
    Color(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b) {}
    
    static Color Black() { return Color(0, 0, 0); }
    static Color White() { return Color(255, 255, 255); }
    static Color Red() { return Color(255, 0, 0); }
    static Color Green() { return Color(0, 255, 0); }
    static Color Blue() { return Color(0, 0, 255); }
};

} // namespace SvgRenderer
