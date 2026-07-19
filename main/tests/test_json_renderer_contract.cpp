/*
 * JsonRenderer Contract Tests
 *
 * Focus: JSON element -> draw command mapping for LVGL rendering.
 */

#include "unity.h"
#include "JsonRenderer.hpp"
#include "JsonTypes.hpp"
#include "FakeDrawTarget.hpp"

using namespace JsonRenderer;

static Screen makeBaseScreen()
{
    Screen s;
    s.title = "contract";
    s.width = 320;
    s.height = 240;
    s.backgroundColor = "#FFFFFF";
    return s;
}

void test_renderer_path_widget_generates_fill_and_stroke_commands(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Widget w;
    w.type = "path";
    w.d = "M 10 10 L 60 10 L 60 40 Z";
    w.fill = "#101010";
    w.strokeColor = "#202020";
    w.strokeWidth = 2.0f;
    screen.widgets.push_back(w);

    TEST_ASSERT_TRUE(renderer.render(screen));
    TEST_ASSERT_GREATER_THAN_INT(0, target.count(DrawCommand::Type::FillHLine));
    TEST_ASSERT_GREATER_THAN_INT(0, target.count(DrawCommand::Type::Line));
}

void test_renderer_label_widget_generates_text_command(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Widget w;
    w.type = "label";
    w.text = "A";
    w.x = 15.0f;
    w.y = 20.0f;
    w.fontSize = 16;
    w.textColor = "#FF0000";
    screen.widgets.push_back(w);

    TEST_ASSERT_TRUE(renderer.render(screen));
    TEST_ASSERT_EQUAL_INT(1, target.count(DrawCommand::Type::Text));

    int idx = target.firstIndex(DrawCommand::Type::Text);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, idx);
    TEST_ASSERT_EQUAL_STRING("A", target.commands[idx].text.c_str());
}

void test_renderer_rect_widget_generates_rect_and_border_lines(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Widget w;
    w.type = "rect";
    w.x = 20.0f;
    w.y = 30.0f;
    w.width = 80.0f;
    w.height = 40.0f;
    w.fillColor = "#FFFFFF";
    w.strokeColor = "#000000";
    w.strokeWidth = 2.0f;
    screen.widgets.push_back(w);

    TEST_ASSERT_TRUE(renderer.render(screen));
    TEST_ASSERT_EQUAL_INT(1, target.count(DrawCommand::Type::Rect));
    TEST_ASSERT_GREATER_OR_EQUAL_INT(4, target.count(DrawCommand::Type::Line));
}

void test_renderer_circle_widget_uses_ellipse_and_clamps_stroke_width(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Widget w;
    w.type = "circle";
    w.x = 120.0f;
    w.y = 90.0f;
    w.rx = 18.0f;
    w.ry = 12.0f;
    w.fillColor = "#00FF00";
    w.strokeColor = "#0000FF";
    w.strokeWidth = 99.0f;
    screen.widgets.push_back(w);

    TEST_ASSERT_TRUE(renderer.render(screen));
    TEST_ASSERT_EQUAL_INT(1, target.count(DrawCommand::Type::Ellipse));

    int idx = target.firstIndex(DrawCommand::Type::Ellipse);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, idx);
    TEST_ASSERT_EQUAL_INT(3, target.commands[idx].strokeWidth);
}

void test_renderer_draw_order_wires_then_widgets_then_ports(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Wire wire;
    wire.id = "w1";
    wire.path = "M 10 10 L 100 10";
    wire.strokeWidth = 2.0f;
    wire.color = "#000000";
    screen.wires.push_back(wire);

    Widget widget;
    widget.type = "rect";
    widget.x = 50.0f;
    widget.y = 40.0f;
    widget.width = 40.0f;
    widget.height = 20.0f;
    widget.fillColor = "#FFFFFF";
    widget.strokeColor = "#000000";
    widget.strokeWidth = 1.0f;
    screen.widgets.push_back(widget);

    Port port;
    port.id = "p1";
    port.x = 150.0f;
    port.y = 70.0f;
    port.radius = 4.0f;
    port.color = "#000000";
    screen.ports.push_back(port);

    TEST_ASSERT_TRUE(renderer.render(screen));

    int firstLine = target.firstIndex(DrawCommand::Type::Line);
    int firstRect = target.firstIndex(DrawCommand::Type::Rect);
    int firstCircle = target.firstIndex(DrawCommand::Type::Circle);

    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, firstLine);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, firstRect);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, firstCircle);
    TEST_ASSERT_TRUE(firstLine < firstRect);
    TEST_ASSERT_TRUE(firstRect < firstCircle);
}

void test_renderer_missing_symbol_is_skipped(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Widget w;
    w.type = "svgSymbol";
    w.symbolId = "does_not_exist";
    w.x = 20.0f;
    w.y = 20.0f;
    screen.widgets.push_back(w);

    TEST_ASSERT_TRUE(renderer.render(screen));

    TEST_ASSERT_EQUAL_INT(0, target.count(DrawCommand::Type::Line));
    TEST_ASSERT_EQUAL_INT(0, target.count(DrawCommand::Type::FillHLine));
    TEST_ASSERT_EQUAL_INT(0, target.count(DrawCommand::Type::Rect));
    TEST_ASSERT_EQUAL_INT(0, target.count(DrawCommand::Type::Circle));
    TEST_ASSERT_EQUAL_INT(0, target.count(DrawCommand::Type::Ellipse));
    TEST_ASSERT_EQUAL_INT(0, target.count(DrawCommand::Type::Text));
}

void test_renderer_wire_stroke_width_is_clamped(void)
{
    FakeDrawTarget target(320, 240);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Wire wire;
    wire.id = "w1";
    wire.path = "M 5 5 L 50 5";
    wire.strokeWidth = 100.0f;
    wire.color = "#000000";
    screen.wires.push_back(wire);

    TEST_ASSERT_TRUE(renderer.render(screen));

    int idx = target.firstIndex(DrawCommand::Type::Line);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, idx);
    TEST_ASSERT_EQUAL_INT(3, target.commands[idx].strokeWidth);
}

void test_renderer_port_radius_is_capped(void)
{
    FakeDrawTarget target(2000, 2000);
    JsonRenderer::JsonRenderer renderer(&target);

    Screen screen = makeBaseScreen();

    Port port;
    port.id = "p1";
    port.x = 150.0f;
    port.y = 100.0f;
    port.radius = 100.0f;
    port.color = "#000000";
    screen.ports.push_back(port);

    TEST_ASSERT_TRUE(renderer.render(screen));

    int idx = target.firstIndex(DrawCommand::Type::Circle);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, idx);
    TEST_ASSERT_EQUAL_INT(8, target.commands[idx].radius);
}
