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

        // Batch all drawing into one layer flush for performance
        m_canvas->beginBatch();

        // Render in order: wires -> junctions -> widgets -> ports
        // (wires should be behind everything)

        renderWires(screen);
        renderJunctions(screen);
        renderWidgets(screen);
        renderPorts(screen);

        m_canvas->endBatch();

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

        // Build debug header with integer-only values (avoid float->string stack overflow)
        char dbuf[512];
        snprintf(dbuf, sizeof(dbuf),
                 "{\"cw\":%d,\"ch\":%d,\"sw\":%d,\"sh\":%d,\"scaleX_pct\":%d,\"scaleY_pct\":%d,\"ox\":%d,\"oy\":%d,\"widgets\":[",
                 (int)m_canvas->width(), (int)m_canvas->height(),
                 (int)screen.width, (int)screen.height,
                 (int)(m_scaleX * 1000), (int)(m_scaleY * 1000),
                 (int)m_offsetX, (int)m_offsetY);
        m_debugInfo = dbuf;
        bool firstWidget = true;

        for (const auto &widget : screen.widgets)
        {
            // --- type="path": render raw SVG path data (baked text, shapes) ---
            if (widget.type == "path")
            {
                if (widget.d.empty())
                {
                    ESP_LOGW(TAG, "Path widget has no 'd' data — skipping");
                    continue;
                }

                SvgRenderer::Color fillColor = parseColor(widget.fill.empty() ? "#000000" : widget.fill);

                // Build a temporary SvgSymbol with the raw path data.
                // viewBox 0,0,1280,800 so path coords map 1:1 to screen coords.
                SvgRenderer::SvgSymbol pathSymbol;
                pathSymbol.id = "baked_path";
                pathSymbol.pathData = widget.d.c_str();
                pathSymbol.viewBox = SvgRenderer::ViewBox(0, 0, (float)screen.width, (float)screen.height);
                pathSymbol.scale = 1.0f;
                pathSymbol.rotation = 0.0f;

                // x=0, y=0: path data already has absolute screen coordinates
                m_svgRenderer->renderSymbolFilled(
                    pathSymbol, 0, 0,
                    fillColor,
                    m_scaleX);

                ESP_LOGD(TAG, "Path widget rendered (d len=%d)", (int)widget.d.size());
                continue;
            }

            // --- type="label": render text using LVGL drawText ---
            if (widget.type == "label")
            {
                if (widget.text.empty())
                {
                    ESP_LOGD(TAG, "Label widget has no text — skipping");
                    continue;
                }

                SvgRenderer::Color textCol = parseColor(widget.textColor.empty() ? "#000000" : widget.textColor);
                LVColor lvTextColor(textCol.r, textCol.g, textCol.b);

                int32_t scaledX = (int32_t)(widget.x * m_scaleX + m_offsetX);
                int32_t scaledY = (int32_t)(widget.y * m_scaleY + m_offsetY);
                int32_t scaledFontSize = (int32_t)(widget.fontSize * m_scaleY);

                m_canvas->drawText(scaledX, scaledY, widget.text.c_str(), lvTextColor, scaledFontSize);

                ESP_LOGD(TAG, "Label widget rendered: \"%s\" size=%d at (%d,%d)",
                         widget.text.c_str(), scaledFontSize, scaledX, scaledY);
                continue;
            }

            if (widget.type != "svgSymbol")
            {
                ESP_LOGW(TAG, "Unsupported widget type: %s", widget.type.c_str());
                continue;
            }

            auto it = screen.embeddedSymbols.find(widget.symbolId);
            if (it == screen.embeddedSymbols.end())
            {
                ESP_LOGW(TAG, "Symbol not found: %s", widget.symbolId.c_str());
                continue;
            }

            const SymbolDef &symbolDef = it->second;

            SvgRenderer::SvgSymbol symbol;
            symbol.id = symbolDef.id.c_str();
            symbol.pathData = symbolDef.pathData.c_str();
            symbol.viewBox = symbolDef.viewBox;
            symbol.scale = widget.scale;
            symbol.rotation = widget.rotation;

            SvgRenderer::Color strokeColor = parseColor(widget.strokeColor);

            // WPF applies position twice: Canvas.SetLeft(widget.X) + TranslateTransform(widget.X,widget.Y)
            // So effective WPF position = 2*widget.X + SVG_point*scale
            int32_t scaledX = (int32_t)(2.0f * widget.x * m_scaleX + m_offsetX);
            int32_t scaledY = (int32_t)(2.0f * widget.y * m_scaleY + m_offsetY);
            int32_t finalScalePct = (int32_t)(widget.scale * m_scaleX * 1000);

            // Verbose debug log (integers only - no float formatting)
            ESP_LOGD(TAG, "WIDGET[%s]: json=(%d,%d) scale_x1000=%d -> canvas=(%d,%d) finalScale_x1000=%d",
                     widget.symbolId.c_str(),
                     (int)widget.x, (int)widget.y, (int)(widget.scale * 1000),
                     scaledX, scaledY, finalScalePct);

            // Accumulate debug JSON (integers only)
            char wbuf[128];
            snprintf(wbuf, sizeof(wbuf),
                     "%s{\"id\":\"%s\",\"jx\":%d,\"jy\":%d,\"cx\":%d,\"cy\":%d,\"fs\":%d}",
                     firstWidget ? "" : ",",
                     widget.symbolId.c_str(),
                     (int)widget.x, (int)widget.y,
                     scaledX, scaledY, finalScalePct);
            m_debugInfo += wbuf;
            firstWidget = false;

            m_svgRenderer->renderSymbol(
                symbol, scaledX, scaledY,
                strokeColor, (int32_t)widget.strokeWidth,
                m_scaleX, widget.rotation);

            if (m_debugMode)
            {
                drawDebugMarker(scaledX, scaledY, widget.symbolId.c_str());
            }
        }
        m_debugInfo += "]}";
        ESP_LOGI(TAG, "Debug: %s", m_debugInfo.c_str());
    }

    void JsonRenderer::drawDebugMarker(int32_t x, int32_t y, const char *label)
    {
        // Red cross at anchor point
        m_canvas->drawLine(x - 8, y, x + 8, y, LVColor(255, 0, 0), 2);
        m_canvas->drawLine(x, y - 8, x, y + 8, LVColor(255, 0, 0), 2);
        // Yellow dot center
        m_canvas->drawCircle(x, y, 4, LVColor(255, 255, 0));
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
