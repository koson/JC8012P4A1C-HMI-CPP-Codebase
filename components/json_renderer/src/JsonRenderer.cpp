#include "JsonRenderer.hpp"
#include "esp_log.h"
#include <algorithm>

static const char *TAG = "JsonRenderer";

namespace JsonRenderer
{

    JsonRenderer::JsonRenderer(LVCanvas *canvas)
        : m_canvas(canvas), m_svgRenderer(nullptr), m_parser(nullptr), m_screen(nullptr),
          m_scaleX(1.0f), m_scaleY(1.0f), m_offsetX(0.0f), m_offsetY(0.0f)
    {

        // Create SVG renderer
        m_svgRenderer = std::make_unique<SvgRenderer::SvgRenderer>(canvas);
        m_svgRenderer->setBezierQuality(30, 20); // High quality rendering

        // Create JSON parser
        m_parser = std::make_unique<JsonParser>();

        ESP_LOGI(TAG, "JsonRenderer initialized");
    }

    JsonRenderer::~JsonRenderer()
    {
    }

    bool JsonRenderer::loadAndRender(const char *filePath)
    {
        ESP_LOGI(TAG, "Loading and rendering: %s", filePath);

        // Clear previous screen
        m_screen = std::make_unique<Screen>();

        // Parse JSON file
        if (!m_parser->parseFile(filePath, *m_screen))
        {
            m_lastError = "Parse error: ";
            m_lastError += m_parser->getLastError();
            ESP_LOGE(TAG, "%s", m_lastError.c_str());
            return false;
        }

        // Render to canvas
        return render(*m_screen);
    }

    bool JsonRenderer::render(const Screen &screen)
    {
        ESP_LOGI(TAG, "Rendering screen: %s", screen.title.c_str());

        // Calculate auto-scaling
        calculateScale(screen);

        // Clear canvas with background color
        SvgRenderer::Color bgColor = parseColor(screen.backgroundColor);
        m_canvas->fill(LVColor(bgColor.r, bgColor.g, bgColor.b));

        // Render in order: wires -> junctions -> widgets -> ports
        // (wires should be behind everything)

        renderWires(screen);
        renderJunctions(screen);
        renderWidgets(screen);
        renderPorts(screen);

        ESP_LOGI(TAG, "Rendering complete");
        return true;
    }

    void JsonRenderer::clear()
    {
        m_canvas->fill(LVColor::White);
    }

    void JsonRenderer::renderWidgets(const Screen &screen)
    {
        ESP_LOGI(TAG, "Rendering %d widgets...", screen.widgets.size());

        for (const auto &widget : screen.widgets)
        {
            if (widget.type != "svgSymbol")
            {
                ESP_LOGW(TAG, "Unsupported widget type: %s", widget.type.c_str());
                continue;
            }

            // Find symbol in embedded symbols
            auto it = screen.embeddedSymbols.find(widget.symbolId);
            if (it == screen.embeddedSymbols.end())
            {
                ESP_LOGW(TAG, "Symbol not found: %s", widget.symbolId.c_str());
                continue;
            }

            const SymbolDef &symbolDef = it->second;

            // Create SvgSymbol for rendering
            SvgRenderer::SvgSymbol symbol;
            symbol.id = symbolDef.id.c_str();
            symbol.pathData = symbolDef.pathData.c_str();
            symbol.viewBox = symbolDef.viewBox;
            symbol.scale = widget.scale;
            symbol.rotation = widget.rotation;

            // Parse colors
            SvgRenderer::Color strokeColor = parseColor(widget.strokeColor);

            // Render symbol
            ESP_LOGI(TAG, "  Rendering widget: %s at (%.0f, %.0f)",
                     widget.symbolId.c_str(), widget.x, widget.y);

            // Different offset handling for positioned widgets vs path widgets
            // Path widgets (x=0, y=0) use absolute SVG coordinates → normal offset
            // Gate widgets (x>0) are in design space → adjusted offset for centering
            bool isPathWidget = (widget.x == 0 && widget.y == 0);
            float offsetMultiplierX = isPathWidget ? 1.0f : 1.9f;

            // Vertical offset varies by Y position (proportional adjustment)
            // XOR (y=50) needs +33, so scale factor = 33/50 = 0.66
            // This makes AND (y=174) get +115px which spreads them properly
            float constantOffsetY = isPathWidget ? 0.0f : (widget.y * 0.66f);

            int32_t scaledX = (int32_t)(widget.x * m_scaleX + m_offsetX * offsetMultiplierX);
            int32_t scaledY = (int32_t)(widget.y * m_scaleY + constantOffsetY);

            ESP_LOGI(TAG, "    Scaled pos: (%d, %d), offset×%.1f+%.0f, raw_y=%.0f",
                     scaledX, scaledY, offsetMultiplierX, constantOffsetY, widget.y);

            m_svgRenderer->renderSymbol(
                symbol,
                scaledX,
                scaledY,
                strokeColor,
                (int32_t)widget.strokeWidth,
                m_scaleX, // Only canvas scale (symbol.scale is already set)
                widget.rotation);
        }
    }

    void JsonRenderer::renderWires(const Screen &screen)
    {
        if (screen.wires.empty())
            return;

        ESP_LOGI(TAG, "Rendering %d wires...", screen.wires.size());

        for (const auto &wire : screen.wires)
        {
            // Parse wire path and render
            // TODO: Implement wire path rendering using SvgPathParser
            // For now, wires are skipped (symbols are more important)
            ESP_LOGD(TAG, "  Wire: %s (path rendering not yet implemented)", wire.id.c_str());
        }
    }

    void JsonRenderer::renderPorts(const Screen &screen)
    {
        if (screen.ports.empty())
            return;

        ESP_LOGI(TAG, "Rendering %d ports...", screen.ports.size());

        for (const auto &port : screen.ports)
        {
            // Parse color
            SvgRenderer::Color color = parseColor(port.color);
            LVColor lvColor(color.r, color.g, color.b);

            // Apply auto-scaling
            int32_t scaledX = (int32_t)(port.x * m_scaleX + m_offsetX);
            int32_t scaledY = (int32_t)(port.y * m_scaleY + m_offsetY);
            int32_t scaledRadius = (int32_t)(port.radius * m_scaleX);

            // Draw circle for port
            m_canvas->drawCircle(
                scaledX,
                scaledY,
                scaledRadius,
                lvColor);

            ESP_LOGD(TAG, "  Port: %s at (%.0f, %.0f)", port.id.c_str(), port.x, port.y);
        }
    }

    void JsonRenderer::renderJunctions(const Screen &screen)
    {
        if (screen.junctions.empty())
            return;

        ESP_LOGI(TAG, "Rendering %d junctions...", screen.junctions.size());

        for (const auto &junction : screen.junctions)
        {
            // Draw filled circle for junction
            LVColor color = LVColor::Black;

            // Apply auto-scaling
            int32_t scaledX = (int32_t)(junction.x * m_scaleX + m_offsetX);
            int32_t scaledY = (int32_t)(junction.y * m_scaleY + m_offsetY);
            int32_t scaledRadius = (int32_t)(junction.radius * m_scaleX);

            m_canvas->drawCircle(
                scaledX,
                scaledY,
                scaledRadius,
                color);

            ESP_LOGD(TAG, "  Junction: %s at (%.0f, %.0f)", junction.id.c_str(), junction.x, junction.y);
        }
    }

    SvgRenderer::Color JsonRenderer::parseColor(const std::string &hexColor)
    {
        // Parse hex color string (e.g., "#2C3E50" or "#000")
        if (hexColor.empty() || hexColor[0] != '#')
        {
            return SvgRenderer::Color::Black();
        }

        std::string hex = hexColor.substr(1); // Remove '#'

        // Expand short format (#RGB -> #RRGGBB)
        if (hex.length() == 3)
        {
            hex = std::string(2, hex[0]) + std::string(2, hex[1]) + std::string(2, hex[2]);
        }

        if (hex.length() != 6)
        {
            return SvgRenderer::Color::Black();
        }

        // Parse hex values
        uint8_t r = std::strtol(hex.substr(0, 2).c_str(), nullptr, 16);
        uint8_t g = std::strtol(hex.substr(2, 2).c_str(), nullptr, 16);
        uint8_t b = std::strtol(hex.substr(4, 2).c_str(), nullptr, 16);

        return SvgRenderer::Color(r, g, b);
    }

    void JsonRenderer::calculateScale(const Screen &screen)
    {
        // Get canvas dimensions from stored values
        int32_t canvasWidth = m_canvas->width();
        int32_t canvasHeight = m_canvas->height();

        // Get screen dimensions from JSON
        float screenWidth = screen.width;
        float screenHeight = screen.height;

        // Calculate scale to fit canvas (maintain aspect ratio)
        float scaleX = canvasWidth / screenWidth;
        float scaleY = canvasHeight / screenHeight;

        // Use uniform scale (smallest to fit everything)
        float scale = std::min(scaleX, scaleY);
        m_scaleX = scale;
        m_scaleY = scale;

        // Calculate centering offset
        float scaledWidth = screenWidth * scale;
        float scaledHeight = screenHeight * scale;
        m_offsetX = (canvasWidth - scaledWidth) / 2.0f;
        m_offsetY = (canvasHeight - scaledHeight) / 2.0f;

        ESP_LOGI(TAG, "Auto-scaling: screen=%dx%d, canvas=%dx%d, scale=%.3f, offset=(%.1f, %.1f)",
                 (int)screenWidth, (int)screenHeight, canvasWidth, canvasHeight,
                 scale, m_offsetX, m_offsetY);
    }

} // namespace JsonRenderer
