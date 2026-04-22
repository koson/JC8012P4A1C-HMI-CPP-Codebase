#pragma once

#include "SvgTypes.hpp"
#include <vector>

namespace SvgRenderer {

/**
 * @brief Bezier Curve to Line Segments Converter
 * 
 * Converts cubic and quadratic Bezier curves into 
 * approximated line segments for rendering.
 */
class BezierConverter {
public:
    /**
     * @brief Convert cubic Bezier curve to line segments
     * 
     * @param p0 Start point
     * @param p1 First control point
     * @param p2 Second control point
     * @param p3 End point
     * @param segments Number of segments to generate (default: 20)
     * @return Vector of points forming the curve
     */
    static std::vector<Point> cubicBezier(
        const Point& p0, 
        const Point& p1, 
        const Point& p2, 
        const Point& p3,
        int segments = 20
    );
    
    /**
     * @brief Convert quadratic Bezier curve to line segments
     * 
     * @param p0 Start point
     * @param p1 Control point
     * @param p2 End point
     * @param segments Number of segments to generate (default: 15)
     * @return Vector of points forming the curve
     */
    static std::vector<Point> quadraticBezier(
        const Point& p0,
        const Point& p1,
        const Point& p2,
        int segments = 15
    );
    
private:
    /**
     * @brief Calculate point on cubic Bezier curve at parameter t
     * @param t Parameter (0.0 to 1.0)
     */
    static Point cubicBezierPoint(
        float t,
        const Point& p0,
        const Point& p1,
        const Point& p2,
        const Point& p3
    );
    
    /**
     * @brief Calculate point on quadratic Bezier curve at parameter t
     * @param t Parameter (0.0 to 1.0)
     */
    static Point quadraticBezierPoint(
        float t,
        const Point& p0,
        const Point& p1,
        const Point& p2
    );
};

} // namespace SvgRenderer
