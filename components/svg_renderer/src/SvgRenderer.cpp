#include "SvgRenderer.hpp"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>

namespace SvgRenderer
{

    SvgRenderer::SvgRenderer(DrawTarget *canvas)
        : m_canvas(canvas), m_cubicBezierSegments(20), m_quadraticBezierSegments(15), m_currentPos(0, 0), m_startPos(0, 0)
    {
    }

    SvgRenderer::~SvgRenderer()
    {
    }

    void SvgRenderer::renderSymbol(
        const SvgSymbol &symbol,
        int32_t x,
        int32_t y,
        const Color &strokeColor,
        int32_t strokeWidth,
        float scaleX,
        float scaleY,
        float rotation)
    {
        if (!m_canvas)
        {
            m_lastError = "Canvas is null";
            return;
        }

        if (!symbol.pathData)
        {
            m_lastError = "Symbol path data is null";
            return;
        }

        // Parse path
        auto commands = m_parser.parse(symbol.pathData);
        if (commands.empty())
        {
            m_lastError = "Failed to parse path: ";
            m_lastError += m_parser.getLastError();
            return;
        }

        // Calculate final scale per axis (symbol scale * additional scale)
        float finalScaleX = symbol.scale * scaleX;
        float finalScaleY = symbol.scale * scaleY;

        // Adjust position for viewBox offset
        int32_t offsetX = x - static_cast<int32_t>(symbol.viewBox.x * finalScaleX);
        int32_t offsetY = y - static_cast<int32_t>(symbol.viewBox.y * finalScaleY);

        // Render path
        renderPath(commands, offsetX, offsetY, finalScaleX, finalScaleY, strokeColor, strokeWidth);
    }

    void SvgRenderer::setBezierQuality(int cubicSegments, int quadraticSegments)
    {
        m_cubicBezierSegments = cubicSegments;
        m_quadraticBezierSegments = quadraticSegments;
    }

    static std::vector<std::pair<std::vector<PathCommand>, bool>> decomposeSubpaths(const std::vector<PathCommand> &commands)
    {
        std::vector<std::pair<std::vector<PathCommand>, bool>> result;
        std::vector<PathCommand> current;
        bool isClosed = false;

        for (const auto &cmd : commands)
        {
            if ((cmd.type == 'M' || cmd.type == 'm') && !current.empty())
            {
                result.push_back({current, isClosed});
                current.clear();
                isClosed = false;
            }
            current.push_back(cmd);
            if (cmd.type == 'Z' || cmd.type == 'z')
            {
                isClosed = true;
            }
        }
        if (!current.empty())
        {
            result.push_back({current, isClosed});
        }
        return result;
    }

    void SvgRenderer::renderPath(
        const std::vector<PathCommand> &commands,
        int32_t offsetX,
        int32_t offsetY,
        float scaleX,
        float scaleY,
        const Color &strokeColor,
        int32_t strokeWidth)
    {
        if (m_canvas->hasVectorSupport())
        {
            auto subpaths = decomposeSubpaths(commands);
            for (const auto &sp : subpaths)
            {
                m_canvas->drawVectorPath(sp.first, offsetX, offsetY, scaleX, scaleY, LVColor::Black, toLVColor(strokeColor), strokeWidth, 0);
            }
            return;
        }

        m_currentPos = Point(0, 0);
        m_startPos = Point(0, 0);

        for (const auto &cmd : commands)
        {
            switch (cmd.type)
            {
            case 'M': // Move to (absolute)
                if (cmd.args.size() >= 2)
                {
                    m_currentPos.x = cmd.args[0];
                    m_currentPos.y = cmd.args[1];
                    m_startPos = m_currentPos;
                }
                break;

            case 'm': // Move to (relative)
                if (cmd.args.size() >= 2)
                {
                    m_currentPos.x += cmd.args[0];
                    m_currentPos.y += cmd.args[1];
                    m_startPos = m_currentPos;
                }
                break;

            case 'L': // Line to (absolute)
                if (cmd.args.size() >= 2)
                {
                    Point target(cmd.args[0], cmd.args[1]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;

            case 'l': // Line to (relative)
                if (cmd.args.size() >= 2)
                {
                    Point target(m_currentPos.x + cmd.args[0], m_currentPos.y + cmd.args[1]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;

            case 'H': // Horizontal line (absolute)
                if (cmd.args.size() >= 1)
                {
                    Point target(cmd.args[0], m_currentPos.y);
                    drawLine(m_currentPos, target, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;

            case 'h': // Horizontal line (relative)
                if (cmd.args.size() >= 1)
                {
                    Point target(m_currentPos.x + cmd.args[0], m_currentPos.y);
                    drawLine(m_currentPos, target, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;

            case 'V': // Vertical line (absolute)
                if (cmd.args.size() >= 1)
                {
                    Point target(m_currentPos.x, cmd.args[0]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;

            case 'v': // Vertical line (relative)
                if (cmd.args.size() >= 1)
                {
                    Point target(m_currentPos.x, m_currentPos.y + cmd.args[0]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;

            case 'C': // Cubic Bezier (absolute)
                if (cmd.args.size() >= 6)
                {
                    Point p1(cmd.args[0], cmd.args[1]);
                    Point p2(cmd.args[2], cmd.args[3]);
                    Point p3(cmd.args[4], cmd.args[5]);
                    drawCubicBezier(m_currentPos, p1, p2, p3, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = p3;
                }
                break;

            case 'c': // Cubic Bezier (relative)
                if (cmd.args.size() >= 6)
                {
                    Point p1(m_currentPos.x + cmd.args[0], m_currentPos.y + cmd.args[1]);
                    Point p2(m_currentPos.x + cmd.args[2], m_currentPos.y + cmd.args[3]);
                    Point p3(m_currentPos.x + cmd.args[4], m_currentPos.y + cmd.args[5]);
                    drawCubicBezier(m_currentPos, p1, p2, p3, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = p3;
                }
                break;

            case 'Q': // Quadratic Bezier (absolute)
                if (cmd.args.size() >= 4)
                {
                    Point p1(cmd.args[0], cmd.args[1]);
                    Point p2(cmd.args[2], cmd.args[3]);
                    drawQuadraticBezier(m_currentPos, p1, p2, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = p2;
                }
                break;

            case 'q': // Quadratic Bezier (relative)
                if (cmd.args.size() >= 4)
                {
                    Point p1(m_currentPos.x + cmd.args[0], m_currentPos.y + cmd.args[1]);
                    Point p2(m_currentPos.x + cmd.args[2], m_currentPos.y + cmd.args[3]);
                    drawQuadraticBezier(m_currentPos, p1, p2, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                    m_currentPos = p2;
                }
                break;

            case 'Z': // Close path (absolute)
            case 'z': // Close path (relative)
                drawLine(m_currentPos, m_startPos, offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
                m_currentPos = m_startPos;
                break;
            }
        }
    }

    void SvgRenderer::drawLine(
        const Point &from,
        const Point &to,
        int32_t offsetX,
        int32_t offsetY,
        float scaleX,
        float scaleY,
        const Color &strokeColor,
        int32_t strokeWidth)
    {
        int32_t x1 = offsetX + static_cast<int32_t>(from.x * scaleX);
        int32_t y1 = offsetY + static_cast<int32_t>(from.y * scaleY);
        int32_t x2 = offsetX + static_cast<int32_t>(to.x * scaleX);
        int32_t y2 = offsetY + static_cast<int32_t>(to.y * scaleY);

        m_canvas->drawLine(x1, y1, x2, y2, toLVColor(strokeColor), strokeWidth);
    }

    void SvgRenderer::drawCubicBezier(
        const Point &p0,
        const Point &p1,
        const Point &p2,
        const Point &p3,
        int32_t offsetX,
        int32_t offsetY,
        float scaleX,
        float scaleY,
        const Color &strokeColor,
        int32_t strokeWidth)
    {
        auto points = BezierConverter::cubicBezier(p0, p1, p2, p3, m_cubicBezierSegments);

        for (size_t i = 1; i < points.size(); i++)
        {
            drawLine(points[i - 1], points[i], offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
        }
    }

    void SvgRenderer::drawQuadraticBezier(
        const Point &p0,
        const Point &p1,
        const Point &p2,
        int32_t offsetX,
        int32_t offsetY,
        float scaleX,
        float scaleY,
        const Color &strokeColor,
        int32_t strokeWidth)
    {
        auto points = BezierConverter::quadraticBezier(p0, p1, p2, m_quadraticBezierSegments);

        for (size_t i = 1; i < points.size(); i++)
        {
            drawLine(points[i - 1], points[i], offsetX, offsetY, scaleX, scaleY, strokeColor, strokeWidth);
        }
    }

    LVColor SvgRenderer::toLVColor(const Color &color)
    {
        return LVColor(color.r, color.g, color.b);
    }

    void SvgRenderer::renderSymbolFilled(
        const SvgSymbol &symbol,
        int32_t x,
        int32_t y,
        const Color &fillColor,
        float scaleX,
        float scaleY)
    {
        if (!m_canvas || !symbol.pathData)
            return;

        auto commands = m_parser.parse(symbol.pathData);
        if (commands.empty())
            return;

        float finalScaleX = symbol.scale * scaleX;
        float finalScaleY = symbol.scale * scaleY;
        int32_t offsetX = x - static_cast<int32_t>(symbol.viewBox.x * finalScaleX);
        int32_t offsetY = y - static_cast<int32_t>(symbol.viewBox.y * finalScaleY);

        renderPathFilled(commands, offsetX, offsetY, finalScaleX, finalScaleY, fillColor);
    }

    void SvgRenderer::renderPathFilled(
        const std::vector<PathCommand> &commands,
        int32_t offsetX,
        int32_t offsetY,
        float scaleX,
        float scaleY,
        const Color &fillColor)
    {
        if (m_canvas->hasVectorSupport())
        {
            auto subpaths = decomposeSubpaths(commands);
            for (const auto &sp : subpaths)
            {
                if (sp.second) // Only fill closed sub-paths (polygons)
                {
                    m_canvas->drawVectorPath(sp.first, offsetX, offsetY, scaleX, scaleY, toLVColor(fillColor), LVColor::Black, 0, LV_OPA_COVER);
                }
            }
            return;
        }

        // Decompose path into closed subpath polygons (M...L...Z groups)
        std::vector<std::vector<Point>> subpaths;
        std::vector<Point> current;
        Point pos(0, 0), start(0, 0);
        bool pathClosed = false;

        auto flush = [&]()
        {
            if (pathClosed && current.size() >= 3)
                subpaths.push_back(current);
            current.clear();
            pathClosed = false;
        };

        float maxScale = std::max(scaleX, scaleY);
        float localFlatness = 0.2f / (maxScale > 0.001f ? maxScale : 1.0f);
        if (localFlatness < 0.01f) {
            localFlatness = 0.01f;
        }

        for (const auto &cmd : commands)
        {
            switch (cmd.type)
            {
            case 'M':
            case 'm':
                flush();
                if (cmd.args.size() >= 2)
                {
                    if (cmd.type == 'M')
                    {
                        pos.x = cmd.args[0];
                        pos.y = cmd.args[1];
                    }
                    else
                    {
                        pos.x += cmd.args[0];
                        pos.y += cmd.args[1];
                    }
                    start = pos;
                    current.push_back(pos);
                }
                break;
            case 'L':
                if (cmd.args.size() >= 2)
                {
                    pos.x = cmd.args[0];
                    pos.y = cmd.args[1];
                    current.push_back(pos);
                }
                break;
            case 'l':
                if (cmd.args.size() >= 2)
                {
                    pos.x += cmd.args[0];
                    pos.y += cmd.args[1];
                    current.push_back(pos);
                }
                break;
            case 'H':
                if (cmd.args.size() >= 1)
                {
                    pos.x = cmd.args[0];
                    current.push_back(pos);
                }
                break;
            case 'h':
                if (cmd.args.size() >= 1)
                {
                    pos.x += cmd.args[0];
                    current.push_back(pos);
                }
                break;
            case 'V':
                if (cmd.args.size() >= 1)
                {
                    pos.y = cmd.args[0];
                    current.push_back(pos);
                }
                break;
            case 'v':
                if (cmd.args.size() >= 1)
                {
                    pos.y += cmd.args[0];
                    current.push_back(pos);
                }
                break;
            case 'C':
                if (cmd.args.size() >= 6)
                {
                    Point p1(cmd.args[0], cmd.args[1]);
                    Point p2(cmd.args[2], cmd.args[3]);
                    Point p3(cmd.args[4], cmd.args[5]);
                    auto pts = BezierConverter::subdivideBezier(pos, p1, p2, p3, localFlatness);
                    for (size_t i = 1; i < pts.size(); i++)
                    {
                        current.push_back(pts[i]);
                    }
                    pos = p3;
                }
                break;
            case 'c':
                if (cmd.args.size() >= 6)
                {
                    Point p1(pos.x + cmd.args[0], pos.y + cmd.args[1]);
                    Point p2(pos.x + cmd.args[2], pos.y + cmd.args[3]);
                    Point p3(pos.x + cmd.args[4], pos.y + cmd.args[5]);
                    auto pts = BezierConverter::subdivideBezier(pos, p1, p2, p3, localFlatness);
                    for (size_t i = 1; i < pts.size(); i++)
                    {
                        current.push_back(pts[i]);
                    }
                    pos = p3;
                }
                break;
            case 'Q':
                if (cmd.args.size() >= 4)
                {
                    Point p1(cmd.args[0], cmd.args[1]);
                    Point p2(cmd.args[2], cmd.args[3]);
                    auto pts = BezierConverter::subdivideBezierQuad(pos, p1, p2, localFlatness);
                    for (size_t i = 1; i < pts.size(); i++)
                    {
                        current.push_back(pts[i]);
                    }
                    pos = p2;
                }
                break;
            case 'q':
                if (cmd.args.size() >= 4)
                {
                    Point p1(pos.x + cmd.args[0], pos.y + cmd.args[1]);
                    Point p2(pos.x + cmd.args[2], pos.y + cmd.args[3]);
                    auto pts = BezierConverter::subdivideBezierQuad(pos, p1, p2, localFlatness);
                    for (size_t i = 1; i < pts.size(); i++)
                    {
                        current.push_back(pts[i]);
                    }
                    pos = p2;
                }
                break;
            case 'Z':
            case 'z':
                current.push_back(start); // close the polygon
                pathClosed = true;
                flush();
                pos = start;
                break;
            default:
                break;
            }
        }
        flush();

        LVColor lv = toLVColor(fillColor);

        if (subpaths.empty())
            return;

        // Global bounding box across ALL subpaths
        float minY = subpaths[0][0].y, maxY = subpaths[0][0].y;
        for (const auto &poly : subpaths)
        {
            for (const auto &p : poly)
            {
                if (p.y < minY)
                    minY = p.y;
                if (p.y > maxY)
                    maxY = p.y;
            }
        }

        int screenMinY = static_cast<int>(minY * scaleY) + offsetY;
        int screenMaxY = static_cast<int>(maxY * scaleY) + offsetY;

        // Pre-allocate xs once outside all loops — avoids heap alloc per scanline
        std::vector<float> xs;
        xs.reserve(32);

        // Scanline fill: even-odd rule applied across ALL subpaths combined.
        // This correctly handles compound paths (e.g. letter 'a' = outer + counter):
        //   outer crosses scanline twice, counter crosses twice → 4 xs total
        //   even-odd fills segments [0,1] and [2,3] — leaving the counter hole empty.
        for (int sy = screenMinY; sy <= screenMaxY; sy++)
        {
            float fy = (sy - offsetY) / scaleY;

            xs.clear(); // reuse allocation, no malloc

            // Collect intersections from every subpath
            for (const auto &poly : subpaths)
            {
                int n = (int)poly.size();
                if (n < 3)
                    continue;
                for (int i = 0; i < n - 1; i++)
                {
                    float y0 = poly[i].y, y1 = poly[i + 1].y;
                    float x0 = poly[i].x, x1 = poly[i + 1].x;
                    if ((y0 <= fy && y1 > fy) || (y1 <= fy && y0 > fy))
                    {
                        float t = (fy - y0) / (y1 - y0);
                        xs.push_back(x0 + t * (x1 - x0));
                    }
                }
            }

            if (xs.size() < 2)
                continue;
            std::sort(xs.begin(), xs.end());
            for (size_t i = 0; i + 1 < xs.size(); i += 2)
            {
                int sx1 = static_cast<int>(xs[i] * scaleX) + offsetX;
                int sx2 = static_cast<int>(xs[i + 1] * scaleX) + offsetX;
                m_canvas->fillHLine(sx1, sx2, sy, lv);
            }
        }

        // Tell LVGL the buffer changed — one invalidation after all subpaths done
        m_canvas->invalidate();
    }

} // namespace SvgRenderer
