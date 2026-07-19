#pragma once

#include "DrawTarget.hpp"
#include <vector>
#include <string>
#include <cstdint>

struct DrawCommand
{
    enum class Type
    {
        Fill,
        BeginBatch,
        EndBatch,
        Rect,
        Line,
        Circle,
        Ellipse,
        Text,
        FillHLine,
        Invalidate
    };

    Type type;
    int32_t x1 = 0;
    int32_t y1 = 0;
    int32_t x2 = 0;
    int32_t y2 = 0;
    int32_t width = 0;
    int32_t radius = 0;
    int32_t rx = 0;
    int32_t ry = 0;
    int32_t strokeWidth = 0;
    int32_t fontSize = 0;
    std::string text;
};

class FakeDrawTarget : public DrawTarget
{
public:
    explicit FakeDrawTarget(uint16_t w = 320, uint16_t h = 240)
        : m_width(w), m_height(h)
    {
    }

    uint16_t width() const override { return m_width; }
    uint16_t height() const override { return m_height; }

    void fill(LVColor color, lv_opa_t opa = LV_OPA_COVER) override
    {
        (void)color;
        (void)opa;
        commands.push_back(DrawCommand{DrawCommand::Type::Fill});
    }

    void beginBatch() override
    {
        commands.push_back(DrawCommand{DrawCommand::Type::BeginBatch});
    }

    void endBatch() override
    {
        commands.push_back(DrawCommand{DrawCommand::Type::EndBatch});
    }

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, LVColor color, lv_opa_t opa = LV_OPA_COVER, int32_t radius = 0) override
    {
        (void)color;
        (void)opa;
        DrawCommand cmd{DrawCommand::Type::Rect};
        cmd.x1 = x;
        cmd.y1 = y;
        cmd.width = w;
        cmd.y2 = h;
        cmd.radius = radius;
        commands.push_back(cmd);
    }

    void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, LVColor color, int32_t width = 2, lv_opa_t opa = LV_OPA_COVER) override
    {
        (void)color;
        (void)opa;
        DrawCommand cmd{DrawCommand::Type::Line};
        cmd.x1 = x1;
        cmd.y1 = y1;
        cmd.x2 = x2;
        cmd.y2 = y2;
        cmd.strokeWidth = width;
        commands.push_back(cmd);
    }

    void drawCircle(int32_t cx, int32_t cy, int32_t radius, LVColor color, bool filled = true, lv_opa_t opa = LV_OPA_COVER) override
    {
        (void)color;
        (void)filled;
        (void)opa;
        DrawCommand cmd{DrawCommand::Type::Circle};
        cmd.x1 = cx;
        cmd.y1 = cy;
        cmd.radius = radius;
        commands.push_back(cmd);
    }

    void drawEllipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, LVColor fillColor, LVColor strokeColor, int32_t strokeWidth = 0, lv_opa_t opa = LV_OPA_COVER) override
    {
        (void)fillColor;
        (void)strokeColor;
        (void)opa;
        DrawCommand cmd{DrawCommand::Type::Ellipse};
        cmd.x1 = cx;
        cmd.y1 = cy;
        cmd.rx = rx;
        cmd.ry = ry;
        cmd.strokeWidth = strokeWidth;
        commands.push_back(cmd);
    }

    void drawText(int32_t x, int32_t y, const char *text, LVColor color, int32_t fontSize = 14, int32_t max_width = 0) override
    {
        (void)color;
        (void)max_width;
        DrawCommand cmd{DrawCommand::Type::Text};
        cmd.x1 = x;
        cmd.y1 = y;
        cmd.fontSize = fontSize;
        cmd.text = text ? text : "";
        commands.push_back(cmd);
    }

    void fillHLine(int32_t x1, int32_t x2, int32_t y, LVColor color) override
    {
        (void)color;
        DrawCommand cmd{DrawCommand::Type::FillHLine};
        cmd.x1 = x1;
        cmd.x2 = x2;
        cmd.y1 = y;
        commands.push_back(cmd);
    }

    void invalidate() override
    {
        commands.push_back(DrawCommand{DrawCommand::Type::Invalidate});
    }

    int count(DrawCommand::Type t) const
    {
        int c = 0;
        for (const auto &cmd : commands)
        {
            if (cmd.type == t)
            {
                c++;
            }
        }
        return c;
    }

    int firstIndex(DrawCommand::Type t) const
    {
        for (size_t i = 0; i < commands.size(); ++i)
        {
            if (commands[i].type == t)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    std::vector<DrawCommand> commands;

private:
    uint16_t m_width;
    uint16_t m_height;
};
