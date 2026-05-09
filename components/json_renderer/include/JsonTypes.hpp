#pragma once

#include <string>
#include <vector>
#include <map>
#include "SvgTypes.hpp"

namespace JsonRenderer
{

    /**
     * @brief Widget definition from JSON
     */
    struct Widget
    {
        std::string symbolId;
        float x;
        float y;
        float scale;
        float rotation;
        std::string fillColor;
        std::string strokeColor;
        float strokeWidth;
        bool flipHorizontal;
        bool flipVertical;
        std::string type; // "svgSymbol", "path", "label", "image", etc.

        // For type="path": SVG path data (d attribute)
        std::string d;
        std::string fill; // fill color for path, e.g. "#000000"

        // Additional properties for labels
        std::string text;
        int fontSize;
        std::string textColor;  // e.g. "#000000"
        std::string fontWeight; // "normal" or "bold"

        Widget()
            : x(0), y(0), scale(1.0f), rotation(0.0f), strokeWidth(2.0f), flipHorizontal(false), flipVertical(false), fontSize(16), textColor("#000000"), fontWeight("normal") {}
    };

    /**
     * @brief Wire definition from JSON
     */
    struct Wire
    {
        std::string id;
        std::string path;
        float strokeWidth;
        std::string color;
        std::string routingStyle;

        Wire() : strokeWidth(2.0f) {}
    };

    /**
     * @brief Port definition from JSON
     */
    struct Port
    {
        std::string id;
        std::string label;
        std::string type; // "input", "output"
        float x;
        float y;
        float radius;
        std::string color;

        Port() : x(0), y(0), radius(5.0f) {}
    };

    /**
     * @brief Junction definition from JSON
     */
    struct Junction
    {
        std::string id;
        float x;
        float y;
        float radius;

        Junction() : x(0), y(0), radius(3.0f) {}
    };

    /**
     * @brief Symbol definition from embedded symbols
     */
    struct SymbolDef
    {
        std::string id;
        std::string title;
        std::string description;
        std::string category;
        std::string pathData;
        SvgRenderer::ViewBox viewBox;

        SymbolDef() : viewBox(0, 0, 150, 150) {}
    };

    /**
     * @brief Complete screen definition from JSON
     */
    struct Screen
    {
        std::string version;
        std::string title;
        std::string description;
        int width;
        int height;
        std::string backgroundColor;

        // Embedded symbols (new in v1.1)
        std::map<std::string, SymbolDef> embeddedSymbols;

        // Optional external symbol library (legacy)
        std::string symbolLibrary;

        // Circuit elements
        std::vector<Widget> widgets;
        std::vector<Wire> wires;
        std::vector<Port> ports;
        std::vector<Junction> junctions;

        Screen() : width(1280), height(800) {}
    };

} // namespace JsonRenderer
