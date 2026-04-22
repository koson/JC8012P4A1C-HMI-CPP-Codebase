#include "BezierConverter.hpp"
#include <cmath>

namespace SvgRenderer {

std::vector<Point> BezierConverter::cubicBezier(
    const Point& p0,
    const Point& p1,
    const Point& p2,
    const Point& p3,
    int segments
) {
    std::vector<Point> points;
    points.reserve(segments + 1);
    
    for (int i = 0; i <= segments; i++) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        Point p = cubicBezierPoint(t, p0, p1, p2, p3);
        points.push_back(p);
    }
    
    return points;
}

std::vector<Point> BezierConverter::quadraticBezier(
    const Point& p0,
    const Point& p1,
    const Point& p2,
    int segments
) {
    std::vector<Point> points;
    points.reserve(segments + 1);
    
    for (int i = 0; i <= segments; i++) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        Point p = quadraticBezierPoint(t, p0, p1, p2);
        points.push_back(p);
    }
    
    return points;
}

Point BezierConverter::cubicBezierPoint(
    float t,
    const Point& p0,
    const Point& p1,
    const Point& p2,
    const Point& p3
) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    Point p;
    p.x = uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x;
    p.y = uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y;
    
    return p;
}

Point BezierConverter::quadraticBezierPoint(
    float t,
    const Point& p0,
    const Point& p1,
    const Point& p2
) {
    float u = 1.0f - t;
    float uu = u * u;
    float tt = t * t;
    
    Point p;
    p.x = uu * p0.x + 2.0f * u * t * p1.x + tt * p2.x;
    p.y = uu * p0.y + 2.0f * u * t * p1.y + tt * p2.y;
    
    return p;
}

} // namespace SvgRenderer
