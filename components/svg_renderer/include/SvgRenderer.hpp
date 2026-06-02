#pragma once

#include "SvgTypes.hpp"
#include "SvgPathParser.hpp"
#include "BezierConverter.hpp"
#include "LVCanvas.hpp"
#include <vector>

namespace SvgRenderer
{

    /**
     * @brief SVG Renderer for LVGL Canvas
     *
     * Renders SVG symbols to LVGL canvas using path parsing
     * and Bezier curve approximation.
     *
     * Example:
     *   SvgRenderer renderer(&canvas);
     *   renderer.renderSymbol(xorGateSymbol, 100, 100, Color::Black, 2);
     */
    class SvgRenderer
    {
    public:
        /**
         * @brief Constructor
         * @param canvas LVGL canvas to render to
         */
        SvgRenderer(LVCanvas *canvas);
        ~SvgRenderer();

        /**
         * @brief Render SVG symbol to canvas
         *
         * @param symbol Symbol definition
         * @param x X position
         * @param y Y position
         * @param strokeColor Stroke color
         * @param strokeWidth Stroke width (default: 2)
         * @param scale Additional scale factor (default: 1.0)
         * @param rotation Additional rotation in degrees (default: 0)
         */
        void renderSymbol(
            const SvgSymbol &symbol,
            int32_t x,
            int32_t y,
            const Color &strokeColor,
            int32_t strokeWidth = 2,
            float scaleX = 1.0f,
            float scaleY = 1.0f,
            float rotation = 0.0f);

        /**
         * @brief Render a symbol using scanline polygon fill (for baked text/shapes).
         * Path data must consist of closed polygons (M...L...Z) only.
         * @param fillColor  Fill colour (e.g. black for text)
         * @param scale      Additional scale on top of symbol.scale
         */
        void renderSymbolFilled(
            const SvgSymbol &symbol,
            int32_t x,
            int32_t y,
            const Color &fillColor,
            float scaleX = 1.0f,
            float scaleY = 1.0f);

        /**
         * @brief Set Bezier curve quality (segments per curve)
         * @param cubicSegments Segments for cubic Bezier (default: 20)
         * @param quadraticSegments Segments for quadratic Bezier (default: 15)
         */
        void setBezierQuality(int cubicSegments, int quadraticSegments);

        /**
         * @brief Get last error message
         */
        const char *getLastError() const { return m_lastError.c_str(); }

    private:
        /**
         * @brief Render parsed path commands (stroke)
         */
        void renderPath(
            const std::vector<PathCommand> &commands,
            int32_t offsetX,
            int32_t offsetY,
            float scaleX,
            float scaleY,
            const Color &strokeColor,
            int32_t strokeWidth);

        /**
         * @brief Scanline fill for closed polygon paths (M...L...Z)
         */
        void renderPathFilled(
            const std::vector<PathCommand> &commands,
            int32_t offsetX,
            int32_t offsetY,
            float scaleX,
            float scaleY,
            const Color &fillColor);

        /**
         * @brief Draw line between two points
         */
        void drawLine(
            const Point &from,
            const Point &to,
            int32_t offsetX,
            int32_t offsetY,
            float scaleX,
            float scaleY,
            const Color &strokeColor,
            int32_t strokeWidth);

        /**
         * @brief Draw cubic Bezier curve
         */
        void drawCubicBezier(
            const Point &p0,
            const Point &p1,
            const Point &p2,
            const Point &p3,
            int32_t offsetX,
            int32_t offsetY,
            float scaleX,
            float scaleY,
            const Color &strokeColor,
            int32_t strokeWidth);

        /**
         * @brief Draw quadratic Bezier curve
         */
        void drawQuadraticBezier(
            const Point &p0,
            const Point &p1,
            const Point &p2,
            int32_t offsetX,
            int32_t offsetY,
            float scaleX,
            float scaleY,
            const Color &strokeColor,
            int32_t strokeWidth);

        /**
         * @brief Convert Color to LVColor
         */
        LVColor toLVColor(const Color &color);

        LVCanvas *m_canvas;
        SvgPathParser m_parser;
        std::string m_lastError;

        int m_cubicBezierSegments;
        int m_quadraticBezierSegments;

        Point m_currentPos;
        Point m_startPos;
    };

} // namespace SvgRenderer
