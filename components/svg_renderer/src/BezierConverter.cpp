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

float BezierConverter::pointToSegmentDistance(const Point& p, const Point& a, const Point& b) {
    float abx = b.x - a.x;
    float aby = b.y - a.y;
    float apx = p.x - a.x;
    float apy = p.y - a.y;
    float ab_len_sq = abx * abx + aby * aby;
    if (ab_len_sq < 1e-6f) {
        return std::sqrt(apx * apx + apy * apy);
    }
    float proj = (apx * abx + apy * aby) / ab_len_sq;
    if (proj < 0.0f) proj = 0.0f;
    if (proj > 1.0f) proj = 1.0f;
    float closest_x = a.x + proj * abx;
    float closest_y = a.y + proj * aby;
    float dx = p.x - closest_x;
    float dy = p.y - closest_y;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<Point> BezierConverter::subdivideBezier(
    const Point& p0,
    const Point& p1,
    const Point& p2,
    const Point& p3,
    float flatness
) {
    std::vector<Point> points;
    points.push_back(p0);
    subdivideBezierRecursive(p0, p1, p2, p3, flatness, points, 0);
    return points;
}

void BezierConverter::subdivideBezierRecursive(
    const Point& p0, const Point& p1, const Point& p2, const Point& p3,
    float flatness, std::vector<Point>& points, int depth
) {
    if (depth > 10) {
        points.push_back(p3);
        return;
    }

    float d1 = pointToSegmentDistance(p1, p0, p3);
    float d2 = pointToSegmentDistance(p2, p0, p3);
    if (d1 <= flatness && d2 <= flatness) {
        points.push_back(p3);
        return;
    }

    // Midpoints
    Point p01((p0.x + p1.x) / 2.0f, (p0.y + p1.y) / 2.0f);
    Point p12((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);
    Point p23((p2.x + p3.x) / 2.0f, (p2.y + p3.y) / 2.0f);

    Point p012((p01.x + p12.x) / 2.0f, (p01.y + p12.y) / 2.0f);
    Point p123((p12.x + p23.x) / 2.0f, (p12.y + p23.y) / 2.0f);

    Point p0123((p012.x + p123.x) / 2.0f, (p012.y + p123.y) / 2.0f);

    subdivideBezierRecursive(p0, p01, p012, p0123, flatness, points, depth + 1);
    subdivideBezierRecursive(p0123, p123, p23, p3, flatness, points, depth + 1);
}

std::vector<Point> BezierConverter::subdivideBezierQuad(
    const Point& p0,
    const Point& p1,
    const Point& p2,
    float flatness
) {
    std::vector<Point> points;
    points.push_back(p0);
    subdivideBezierQuadRecursive(p0, p1, p2, flatness, points, 0);
    return points;
}

void BezierConverter::subdivideBezierQuadRecursive(
    const Point& p0, const Point& p1, const Point& p2,
    float flatness, std::vector<Point>& points, int depth
) {
    if (depth > 10) {
        points.push_back(p2);
        return;
    }

    float d = pointToSegmentDistance(p1, p0, p2);
    if (d <= flatness) {
        points.push_back(p2);
        return;
    }

    // Midpoints
    Point p01((p0.x + p1.x) / 2.0f, (p0.y + p1.y) / 2.0f);
    Point p12((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);

    Point p012((p01.x + p12.x) / 2.0f, (p01.y + p12.y) / 2.0f);

    subdivideBezierQuadRecursive(p0, p01, p012, flatness, points, depth + 1);
    subdivideBezierQuadRecursive(p012, p12, p2, flatness, points, depth + 1);
}

} // namespace SvgRenderer
