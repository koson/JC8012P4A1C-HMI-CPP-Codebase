#include "unity.h"
#include "BezierConverter.hpp"
#include "SvgRenderer.hpp"
#include "FakeDrawTarget.hpp"
#include "esp_log.h"
#include <vector>

using namespace SvgRenderer;

void test_bezier_adaptive_flat_line(void)
{
    // A completely flat cubic Bezier curve (p0, p1, p2, p3 are colinear)
    Point p0(0.0f, 0.0f);
    Point p1(10.0f, 0.0f);
    Point p2(20.0f, 0.0f);
    Point p3(30.0f, 0.0f);

    // With a flatness tolerance of 0.2, it should detect that it's already flat
    // and return only the start and end points (or very few points)
    auto points = BezierConverter::subdivideBezier(p0, p1, p2, p3, 0.2f);

    TEST_ASSERT_EQUAL_UINT32(2, points.size());
    TEST_ASSERT_EQUAL_FLOAT(p0.x, points.front().x);
    TEST_ASSERT_EQUAL_FLOAT(p0.y, points.front().y);
    TEST_ASSERT_EQUAL_FLOAT(p3.x, points.back().x);
    TEST_ASSERT_EQUAL_FLOAT(p3.y, points.back().y);
}

void test_bezier_adaptive_curve_points(void)
{
    // A highly curved cubic Bezier curve
    Point p0(0.0f, 0.0f);
    Point p1(10.0f, 20.0f);
    Point p2(20.0f, -20.0f);
    Point p3(30.0f, 0.0f);

    auto points = BezierConverter::subdivideBezier(p0, p1, p2, p3, 0.2f);

    // It should subdivide and generate multiple points to maintain flatness
    TEST_ASSERT_GREATER_THAN_UINT32(2, points.size());
    TEST_ASSERT_EQUAL_FLOAT(p0.x, points.front().x);
    TEST_ASSERT_EQUAL_FLOAT(p0.y, points.front().y);
    TEST_ASSERT_EQUAL_FLOAT(p3.x, points.back().x);
    TEST_ASSERT_EQUAL_FLOAT(p3.y, points.back().y);
}

void test_bezier_quad_adaptive_curve(void)
{
    Point p0(0.0f, 0.0f);
    Point p1(15.0f, 15.0f);
    Point p2(30.0f, 0.0f);

    auto points = BezierConverter::subdivideBezierQuad(p0, p1, p2, 0.2f);

    TEST_ASSERT_GREATER_THAN_UINT32(2, points.size());
    TEST_ASSERT_EQUAL_FLOAT(p0.x, points.front().x);
    TEST_ASSERT_EQUAL_FLOAT(p0.y, points.front().y);
    TEST_ASSERT_EQUAL_FLOAT(p2.x, points.back().x);
    TEST_ASSERT_EQUAL_FLOAT(p2.y, points.back().y);
}

void test_renderer_bezier_filled_or_gate(void)
{
    FakeDrawTarget target(320, 240);
    SvgRenderer::SvgRenderer renderer(&target);

    // Simple closed path with cubic curves resembling a gate
    SvgSymbol sym;
    sym.id = "test_or";
    sym.pathData = "M 10 10 C 25 25 25 55 10 70 L 60 70 C 80 40 80 40 60 10 Z";
    sym.viewBox = ViewBox(0, 0, 100, 100);
    sym.scale = 1.0f;
    sym.rotation = 0.0f;

    // Render filled
    renderer.renderSymbolFilled(sym, 10, 10, Color(255, 0, 0), 1.0f, 1.0f);

    // Should call fillHLine several times to render the filled region
    TEST_ASSERT_GREATER_THAN_INT(0, target.count(DrawCommand::Type::FillHLine));
}
