#include "SvgRenderer.hpp"
#include <cmath>

namespace SvgRenderer {

SvgRenderer::SvgRenderer(LVCanvas* canvas)
    : m_canvas(canvas)
    , m_cubicBezierSegments(20)
    , m_quadraticBezierSegments(15)
    , m_currentPos(0, 0)
    , m_startPos(0, 0) {
}

SvgRenderer::~SvgRenderer() {
}

void SvgRenderer::renderSymbol(
    const SvgSymbol& symbol,
    int32_t x,
    int32_t y,
    const Color& strokeColor,
    int32_t strokeWidth,
    float scale,
    float rotation
) {
    if (!m_canvas) {
        m_lastError = "Canvas is null";
        return;
    }
    
    if (!symbol.pathData) {
        m_lastError = "Symbol path data is null";
        return;
    }
    
    // Parse path
    auto commands = m_parser.parse(symbol.pathData);
    if (commands.empty()) {
        m_lastError = "Failed to parse path: ";
        m_lastError += m_parser.getLastError();
        return;
    }
    
    // Calculate final scale (symbol scale * additional scale)
    float finalScale = symbol.scale * scale;
    
    // Adjust position for viewBox offset
    int32_t offsetX = x - static_cast<int32_t>(symbol.viewBox.x * finalScale);
    int32_t offsetY = y - static_cast<int32_t>(symbol.viewBox.y * finalScale);
    
    // Render path
    renderPath(commands, offsetX, offsetY, finalScale, strokeColor, strokeWidth);
}

void SvgRenderer::setBezierQuality(int cubicSegments, int quadraticSegments) {
    m_cubicBezierSegments = cubicSegments;
    m_quadraticBezierSegments = quadraticSegments;
}

void SvgRenderer::renderPath(
    const std::vector<PathCommand>& commands,
    int32_t offsetX,
    int32_t offsetY,
    float scale,
    const Color& strokeColor,
    int32_t strokeWidth
) {
    m_currentPos = Point(0, 0);
    m_startPos = Point(0, 0);
    
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case 'M': // Move to (absolute)
                if (cmd.args.size() >= 2) {
                    m_currentPos.x = cmd.args[0];
                    m_currentPos.y = cmd.args[1];
                    m_startPos = m_currentPos;
                }
                break;
                
            case 'm': // Move to (relative)
                if (cmd.args.size() >= 2) {
                    m_currentPos.x += cmd.args[0];
                    m_currentPos.y += cmd.args[1];
                    m_startPos = m_currentPos;
                }
                break;
                
            case 'L': // Line to (absolute)
                if (cmd.args.size() >= 2) {
                    Point target(cmd.args[0], cmd.args[1]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;
                
            case 'l': // Line to (relative)
                if (cmd.args.size() >= 2) {
                    Point target(m_currentPos.x + cmd.args[0], m_currentPos.y + cmd.args[1]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;
                
            case 'H': // Horizontal line (absolute)
                if (cmd.args.size() >= 1) {
                    Point target(cmd.args[0], m_currentPos.y);
                    drawLine(m_currentPos, target, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;
                
            case 'h': // Horizontal line (relative)
                if (cmd.args.size() >= 1) {
                    Point target(m_currentPos.x + cmd.args[0], m_currentPos.y);
                    drawLine(m_currentPos, target, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;
                
            case 'V': // Vertical line (absolute)
                if (cmd.args.size() >= 1) {
                    Point target(m_currentPos.x, cmd.args[0]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;
                
            case 'v': // Vertical line (relative)
                if (cmd.args.size() >= 1) {
                    Point target(m_currentPos.x, m_currentPos.y + cmd.args[0]);
                    drawLine(m_currentPos, target, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = target;
                }
                break;
                
            case 'C': // Cubic Bezier (absolute)
                if (cmd.args.size() >= 6) {
                    Point p1(cmd.args[0], cmd.args[1]);
                    Point p2(cmd.args[2], cmd.args[3]);
                    Point p3(cmd.args[4], cmd.args[5]);
                    drawCubicBezier(m_currentPos, p1, p2, p3, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = p3;
                }
                break;
                
            case 'c': // Cubic Bezier (relative)
                if (cmd.args.size() >= 6) {
                    Point p1(m_currentPos.x + cmd.args[0], m_currentPos.y + cmd.args[1]);
                    Point p2(m_currentPos.x + cmd.args[2], m_currentPos.y + cmd.args[3]);
                    Point p3(m_currentPos.x + cmd.args[4], m_currentPos.y + cmd.args[5]);
                    drawCubicBezier(m_currentPos, p1, p2, p3, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = p3;
                }
                break;
                
            case 'Q': // Quadratic Bezier (absolute)
                if (cmd.args.size() >= 4) {
                    Point p1(cmd.args[0], cmd.args[1]);
                    Point p2(cmd.args[2], cmd.args[3]);
                    drawQuadraticBezier(m_currentPos, p1, p2, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = p2;
                }
                break;
                
            case 'q': // Quadratic Bezier (relative)
                if (cmd.args.size() >= 4) {
                    Point p1(m_currentPos.x + cmd.args[0], m_currentPos.y + cmd.args[1]);
                    Point p2(m_currentPos.x + cmd.args[2], m_currentPos.y + cmd.args[3]);
                    drawQuadraticBezier(m_currentPos, p1, p2, offsetX, offsetY, scale, strokeColor, strokeWidth);
                    m_currentPos = p2;
                }
                break;
                
            case 'Z': // Close path (absolute)
            case 'z': // Close path (relative)
                drawLine(m_currentPos, m_startPos, offsetX, offsetY, scale, strokeColor, strokeWidth);
                m_currentPos = m_startPos;
                break;
        }
    }
}

void SvgRenderer::drawLine(
    const Point& from,
    const Point& to,
    int32_t offsetX,
    int32_t offsetY,
    float scale,
    const Color& strokeColor,
    int32_t strokeWidth
) {
    int32_t x1 = offsetX + static_cast<int32_t>(from.x * scale);
    int32_t y1 = offsetY + static_cast<int32_t>(from.y * scale);
    int32_t x2 = offsetX + static_cast<int32_t>(to.x * scale);
    int32_t y2 = offsetY + static_cast<int32_t>(to.y * scale);
    
    m_canvas->drawLine(x1, y1, x2, y2, toLVColor(strokeColor), strokeWidth);
}

void SvgRenderer::drawCubicBezier(
    const Point& p0,
    const Point& p1,
    const Point& p2,
    const Point& p3,
    int32_t offsetX,
    int32_t offsetY,
    float scale,
    const Color& strokeColor,
    int32_t strokeWidth
) {
    auto points = BezierConverter::cubicBezier(p0, p1, p2, p3, m_cubicBezierSegments);
    
    for (size_t i = 1; i < points.size(); i++) {
        drawLine(points[i-1], points[i], offsetX, offsetY, scale, strokeColor, strokeWidth);
    }
}

void SvgRenderer::drawQuadraticBezier(
    const Point& p0,
    const Point& p1,
    const Point& p2,
    int32_t offsetX,
    int32_t offsetY,
    float scale,
    const Color& strokeColor,
    int32_t strokeWidth
) {
    auto points = BezierConverter::quadraticBezier(p0, p1, p2, m_quadraticBezierSegments);
    
    for (size_t i = 1; i < points.size(); i++) {
        drawLine(points[i-1], points[i], offsetX, offsetY, scale, strokeColor, strokeWidth);
    }
}

LVColor SvgRenderer::toLVColor(const Color& color) {
    return LVColor(color.r, color.g, color.b);
}

} // namespace SvgRenderer
