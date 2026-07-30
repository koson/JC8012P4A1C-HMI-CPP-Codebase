#include "JsonRenderer.hpp"
#include "esp_log.h"
#include <algorithm>
#include <cmath>
#include <limits>

static const char *TAG = "JsonRenderer";

namespace JsonRenderer
{
    namespace
    {
        // Visual policy: geometry scales with viewport, stroke width does not.
        // Keep these limits aligned with CircuitCanvas (WASM) for parity.
        constexpr int32_t kMinStrokePx = 1;
        constexpr int32_t kMaxStrokePx = 3;     // cap at 3px so wires stay thin at all scales
        constexpr int32_t kMaxPortRadiusPx = 8; // port circles never exceed 8px regardless of scale
        // Gate size policy: a single svgSymbol widget must not render taller than this.
        // Ensures single-gate lessons look similar in size to multi-gate lessons.
        constexpr float kMaxGateScreenPx = 280.0f;

        inline int32_t clampStrokePx(float stroke)
        {
            float safe = (stroke > 0.0f) ? stroke : 1.0f;
            int32_t px = static_cast<int32_t>(safe + 0.5f);
            return std::clamp(px, kMinStrokePx, kMaxStrokePx);
        }
    }

        JsonRenderer::JsonRenderer(DrawTarget *canvas)
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

    bool JsonRenderer::renderJsonObj(cJSON *root)
    {
        ESP_LOGI(TAG, "Rendering pre-parsed cJSON object");

        // Clear previous screen
        m_screen = std::make_unique<Screen>();

        // Parse JSON object
        if (!m_parser->parseJsonObj(root, *m_screen))
        {
            m_lastError = "Parse error: ";
            m_lastError += m_parser->getLastError();
            ESP_LOGE(TAG, "%s", m_lastError.c_str());
            return false;
        }

        // Render to canvas
        return render(*m_screen);
    }

    bool JsonRenderer::renderJsonString(const char *jsonStr)
    {
        ESP_LOGI(TAG, "Rendering JSON string");

        if (!jsonStr || !jsonStr[0])
        {
            m_lastError = "JSON string is empty";
            return false;
        }

        m_screen = std::make_unique<Screen>();

        if (!m_parser->parseString(jsonStr, *m_screen))
        {
            m_lastError = "Parse error: ";
            m_lastError += m_parser->getLastError();
            ESP_LOGE(TAG, "%s", m_lastError.c_str());
            return false;
        }

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

                // Build a temporary SvgSymbol with the raw path data.
                // viewBox 0,0,1280,800 so path coords map 1:1 to screen coords.
                SvgRenderer::SvgSymbol pathSymbol;
                pathSymbol.id = "baked_path";
                pathSymbol.pathData = widget.d.c_str();
                pathSymbol.viewBox = SvgRenderer::ViewBox(0, 0, (float)screen.width, (float)screen.height);
                pathSymbol.scale = 1.0f;
                pathSymbol.rotation = 0.0f;

                bool isFillNone = (widget.fill == "none" || widget.fill.empty());
                bool hasStroke = !widget.strokeColor.empty() && widget.strokeColor != "none" && widget.strokeWidth > 0.0f;

                // Pass 1: fill (skip if fill=none or fill=white on white bg — open paths)
                if (!isFillNone)
                {
                    SvgRenderer::Color fillColor = parseColor(widget.fill);
                    m_svgRenderer->renderSymbolFilled(
                        pathSymbol, 0, 0,
                        fillColor,
                        m_scaleX,
                        m_scaleY);
                }

                // Pass 2: stroke (always render when strokeColor is set, independent of fill)
                if (hasStroke)
                {
                    SvgRenderer::Color strokeColor = parseColor(widget.strokeColor);
                    int32_t sw = clampStrokePx(widget.strokeWidth > 0 ? widget.strokeWidth : 2.0f);
                    m_svgRenderer->renderSymbol(
                        pathSymbol, 0, 0,
                        strokeColor, sw,
                        m_scaleX, m_scaleY, 0.0f);
                }

                // Fallback: if neither fill nor stroke, use stroke with default color
                if (isFillNone && !hasStroke)
                {
                    SvgRenderer::Color strokeColor = parseColor("#000000");
                    m_svgRenderer->renderSymbol(pathSymbol, 0, 0, strokeColor, 1, m_scaleX, m_scaleY, 0.0f);
                }

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

                ESP_LOGI(TAG, "Label widget rendered: \"%s\" size=%d at (%d,%d)",
                         widget.text.c_str(), scaledFontSize, scaledX, scaledY);
                continue;
            }

            // --- type="rect": draw filled rectangle with stroke outline ---
            if (widget.type == "rect")
            {
                int32_t scaledX = (int32_t)(widget.x * m_scaleX + m_offsetX);
                int32_t scaledY = (int32_t)(widget.y * m_scaleY + m_offsetY);
                int32_t scaledW = (int32_t)(widget.width * m_scaleX);
                int32_t scaledH = (int32_t)(widget.height * m_scaleY);

                SvgRenderer::Color fillCol = parseColor(widget.fillColor.empty() ? "#FFFFFF" : widget.fillColor);
                LVColor lvFill(fillCol.r, fillCol.g, fillCol.b);
                m_canvas->drawRect(scaledX, scaledY, scaledW, scaledH, lvFill);

                if (widget.strokeWidth > 0 && !widget.strokeColor.empty())
                {
                    SvgRenderer::Color strokeCol = parseColor(widget.strokeColor);
                    LVColor lvStroke(strokeCol.r, strokeCol.g, strokeCol.b);
                    int32_t sw = clampStrokePx(widget.strokeWidth);
                    m_canvas->drawLine(scaledX, scaledY, scaledX + scaledW, scaledY, lvStroke, sw);
                    m_canvas->drawLine(scaledX + scaledW, scaledY, scaledX + scaledW, scaledY + scaledH, lvStroke, sw);
                    m_canvas->drawLine(scaledX + scaledW, scaledY + scaledH, scaledX, scaledY + scaledH, lvStroke, sw);
                    m_canvas->drawLine(scaledX, scaledY + scaledH, scaledX, scaledY, lvStroke, sw);
                }

                ESP_LOGD(TAG, "Rect widget rendered at (%d,%d) size %dx%d", scaledX, scaledY, scaledW, scaledH);
                continue;
            }

            // --- type="circle": draw filled circle ---
            if (widget.type == "circle")
            {
                int32_t scaledX = (int32_t)(widget.x * m_scaleX + m_offsetX);
                int32_t scaledY = (int32_t)(widget.y * m_scaleY + m_offsetY);

                // Use rx/ry (derived from width/height) for true ellipse support
                int32_t scaledRx = (int32_t)(widget.rx * m_scaleX);
                int32_t scaledRy = (int32_t)(widget.ry * m_scaleY);

                SvgRenderer::Color fillCol = parseColor(widget.fillColor.empty() ? "#FFFFFF" : widget.fillColor);
                SvgRenderer::Color strokeCol = parseColor(widget.strokeColor.empty() ? "#000000" : widget.strokeColor);
                LVColor lvFill(fillCol.r, fillCol.g, fillCol.b);
                LVColor lvStroke(strokeCol.r, strokeCol.g, strokeCol.b);

                int32_t sw = (widget.strokeWidth > 0.0f) ? clampStrokePx(widget.strokeWidth) : 0;
                m_canvas->drawEllipse(scaledX, scaledY, scaledRx, scaledRy, lvFill, lvStroke, sw);

                ESP_LOGD(TAG, "Ellipse widget rendered at (%d,%d) rx=%d ry=%d sw=%d", scaledX, scaledY, scaledRx, scaledRy, sw);
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

            // Coordinate mode marker from converter:
            // - coord_mode=raw: widget x/y are already in canvas space
            // - legacy exports: keep historical x2 compensation
            const bool rawCoords =
                (screen.description.find("coord_mode=raw") != std::string::npos);
            const bool drawioHalfCoords = !rawCoords &&
                                          (screen.description.find("DrawioToCircuitConverter") != std::string::npos);
            const float widgetBaseX = drawioHalfCoords ? (widget.x * 2.0f) : widget.x;
            const float widgetBaseY = drawioHalfCoords ? (widget.y * 2.0f) : widget.y;

            SvgRenderer::SvgSymbol symbol;
            symbol.id = symbolDef.id.c_str();
            symbol.pathData = symbolDef.pathData.c_str();
            symbol.viewBox = symbolDef.viewBox;
            symbol.scale = widget.scale;
            symbol.rotation = widget.rotation;

            SvgRenderer::Color fillColor = parseColor(widget.fillColor.empty() ? "#ECF0F1" : widget.fillColor);
            SvgRenderer::Color strokeColor = parseColor(widget.strokeColor);

            // Base placement in the same coordinate space as wires/ports.
            float placeX = widgetBaseX;
            float placeY = widgetBaseY;
            float logicalScaleX = (widget.scale > 0.0f) ? widget.scale : 1.0f;
            float logicalScaleY = (widget.scale > 0.0f) ? widget.scale : 1.0f;

            // Draw.io exporter usually provides width/height + viewBox + scale=1.
            // Use viewBox fitting first for stable cross-symbol sizing.
            if (widget.width > 0.0f && widget.height > 0.0f)
            {
                const float vbW = symbol.viewBox.width;
                const float vbH = symbol.viewBox.height;

                if (vbW > 0.0f && vbH > 0.0f)
                {
                    const float boundsW = vbW;
                    const float boundsH = vbH;
                    float minX = symbol.viewBox.x;
                    float minY = symbol.viewBox.y;

                    // If pathData has baked absolute coordinates (e.g. minX=70) while viewBox.x was 0,
                    // compute actual path minX to prevent double-offset shift
                    if (symbol.pathData && symbol.pathData[0] != '\0')
                    {
                        SvgRenderer::SvgPathParser parser;
                        const auto commands = parser.parse(symbol.pathData);
                        float pathMinX = std::numeric_limits<float>::infinity();
                        float pathMinY = std::numeric_limits<float>::infinity();
                        float cx = 0.0f, cy = 0.0f;
                        for (const auto &cmd : commands)
                        {
                            bool isRel = (cmd.type >= 'a' && cmd.type <= 'z');
                            for (size_t i = 0; i + 1 < cmd.args.size(); i += 2)
                            {
                                float px = (isRel ? cx : 0.0f) + cmd.args[i];
                                float py = (isRel ? cy : 0.0f) + cmd.args[i + 1];
                                pathMinX = std::min(pathMinX, px);
                                pathMinY = std::min(pathMinY, py);
                            }
                            if (!cmd.args.empty() && cmd.args.size() >= 2)
                            {
                                cx = (isRel ? cx : 0.0f) + cmd.args[cmd.args.size() - 2];
                                cy = (isRel ? cy : 0.0f) + cmd.args[cmd.args.size() - 1];
                            }
                        }
                        if (std::isfinite(pathMinX) && pathMinX > 0.0f && symbol.viewBox.x == 0.0f && pathMinX >= widgetBaseX)
                        {
                            minX = pathMinX;
                        }
                        if (std::isfinite(pathMinY) && pathMinY > 0.0f && symbol.viewBox.y == 0.0f && pathMinY >= widgetBaseY)
                        {
                            minY = pathMinY;
                        }
                    }

                    // Scale uniformly based on width to align pins with wires
                    const float uniformScale = widget.width / boundsW;
                    logicalScaleX = uniformScale;
                    logicalScaleY = uniformScale;

                    // Place symbol directly at widgetBaseX, widgetBaseY (matching WASM & DrawIO spec)
                    // Do NOT add extraW / 2.0f centering offset, as that shifts gate pins away from wire endpoints!
                    placeX = widgetBaseX;
                    placeY = widgetBaseY;

                    // Detailed log showing SVG coordinates vs logical HMI coordinates
                    ESP_LOGI(TAG, "=== SVG COORD DIAGNOSTIC [%s] ===", widget.symbolId.c_str());
                    ESP_LOGI(TAG, "  [RAW JSON] path_data: \"%s\"", symbol.pathData ? symbol.pathData : "");
                    ESP_LOGI(TAG, "  [RAW JSON] viewBox: (minX=%.1f, minY=%.1f, w=%.1f, h=%.1f)", minX, minY, boundsW, boundsH);
                    ESP_LOGI(TAG, "  [RAW JSON] widget: pos=(%.1f, %.1f), size=(%.1f, %.1f)", widget.x, widget.y, widget.width, widget.height);
                    ESP_LOGI(TAG, "  [CANVAS COMPUTE] placeX=%.1f (baseX=%.1f - minX=%.1f * scale=%.3f)", placeX, widgetBaseX, minX, uniformScale);
                    ESP_LOGI(TAG, "  [CANVAS COMPUTE] placeY=%.1f (baseY=%.1f - minY=%.1f * scale=%.3f)", placeY, widgetBaseY, minY, uniformScale);
                }
                else if (symbol.pathData)
                {
                    // Fallback for legacy symbols without a valid viewBox.
                    SvgRenderer::SvgPathParser parser;
                    const auto commands = parser.parse(symbol.pathData);
                    float minX = std::numeric_limits<float>::infinity();
                    float minY = std::numeric_limits<float>::infinity();
                    float maxX = -std::numeric_limits<float>::infinity();
                    float maxY = -std::numeric_limits<float>::infinity();
                    float cx = 0.0f;
                    float cy = 0.0f;

                    for (const auto &cmd : commands)
                    {
                        auto includePoint = [&](float px, float py)
                        {
                            if (px < minX)
                                minX = px;
                            if (py < minY)
                                minY = py;
                            if (px > maxX)
                                maxX = px;
                            if (py > maxY)
                                maxY = py;
                        };

                        switch (cmd.type)
                        {
                        case 'M':
                        case 'L':
                            if (cmd.args.size() >= 2)
                            {
                                cx = cmd.args[0];
                                cy = cmd.args[1];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'm':
                        case 'l':
                            if (cmd.args.size() >= 2)
                            {
                                cx += cmd.args[0];
                                cy += cmd.args[1];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'H':
                            if (cmd.args.size() >= 1)
                            {
                                cx = cmd.args[0];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'h':
                            if (cmd.args.size() >= 1)
                            {
                                cx += cmd.args[0];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'V':
                            if (cmd.args.size() >= 1)
                            {
                                cy = cmd.args[0];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'v':
                            if (cmd.args.size() >= 1)
                            {
                                cy += cmd.args[0];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'C':
                            if (cmd.args.size() >= 6)
                            {
                                includePoint(cmd.args[0], cmd.args[1]);
                                includePoint(cmd.args[2], cmd.args[3]);
                                cx = cmd.args[4];
                                cy = cmd.args[5];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'c':
                            if (cmd.args.size() >= 6)
                            {
                                includePoint(cx + cmd.args[0], cy + cmd.args[1]);
                                includePoint(cx + cmd.args[2], cy + cmd.args[3]);
                                cx += cmd.args[4];
                                cy += cmd.args[5];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'Q':
                            if (cmd.args.size() >= 4)
                            {
                                includePoint(cmd.args[0], cmd.args[1]);
                                cx = cmd.args[2];
                                cy = cmd.args[3];
                                includePoint(cx, cy);
                            }
                            break;
                        case 'q':
                            if (cmd.args.size() >= 4)
                            {
                                includePoint(cx + cmd.args[0], cy + cmd.args[1]);
                                cx += cmd.args[2];
                                cy += cmd.args[3];
                                includePoint(cx, cy);
                            }
                            break;
                        default:
                            break;
                        }
                    }

                    const bool hasBounds = std::isfinite(minX) && std::isfinite(minY) && std::isfinite(maxX) && std::isfinite(maxY) &&
                                           (maxX > minX) && (maxY > minY);
                    if (hasBounds)
                    {
                        const float boundsW = maxX - minX;
                        const float boundsH = maxY - minY;
                        
                        // Scale uniformly based on width to align pins with wires
                        const float uniformScale = widget.width / boundsW;
                        logicalScaleX = uniformScale;
                        logicalScaleY = uniformScale;
                        
                        // Center the symbol vertically within the widget's bounding box
                        const float extraW = widget.width - boundsW * uniformScale;
                        const float extraH = widget.height - boundsH * uniformScale;
                        
                        placeX = widgetBaseX - minX * uniformScale + extraW / 2.0f;
                        placeY = widgetBaseY - minY * uniformScale + extraH / 2.0f;

                        // Detailed log showing SVG coordinates vs logical HMI coordinates
                        ESP_LOGI(TAG, "SVG Layout scaling for [%s]:", widget.symbolId.c_str());
                        ESP_LOGI(TAG, "  - SVG symbol path bounds: min=(%.1f, %.1f), max=(%.1f, %.1f), size=(%.1f, %.1f)", 
                                 minX, minY, maxX, maxY, boundsW, boundsH);
                        ESP_LOGI(TAG, "  - HMI JSON widget box: pos=(%.1f, %.1f), size=(%.1f, %.1f)", 
                                 widget.x, widget.y, widget.width, widget.height);
                        ESP_LOGI(TAG, "  - Scaling: UniformScale (width-based) = %.3f", 
                                 uniformScale);
                        ESP_LOGI(TAG, "  - Centering padding: padX=%.1f, padY=%.1f", 
                                 extraW / 2.0f, extraH / 2.0f);
                    }
                }
            }

            int32_t scaledX = (int32_t)(placeX * m_scaleX + m_offsetX);
            int32_t scaledY = (int32_t)(placeY * m_scaleY + m_offsetY);
            int32_t finalScaleXPct = (int32_t)(logicalScaleX * m_scaleX * 1000);
            int32_t finalScaleYPct = (int32_t)(logicalScaleY * m_scaleY * 1000);

            // Log final screen pixel coordinates (LVGL space)
            ESP_LOGI(TAG, "  - Final LVGL screen placement: canvas_origin=(%d, %d), final_scale=(%.3f, %.3f)",
                     (int)scaledX, (int)scaledY,
                     logicalScaleX * m_scaleX, logicalScaleY * m_scaleY);

            // Verbose debug log (integers only - no float formatting)
            ESP_LOGD(TAG, "WIDGET[%s]: json=(%d,%d) box=(%d,%d) scale_x1000=(%d,%d) -> canvas=(%d,%d)",
                     widget.symbolId.c_str(),
                     (int)widget.x, (int)widget.y, (int)widget.width, (int)widget.height,
                     (int)(logicalScaleX * 1000), (int)(logicalScaleY * 1000),
                     scaledX, scaledY);

            // Accumulate debug JSON (integers only)
            char wbuf[160];
            snprintf(wbuf, sizeof(wbuf),
                     "%s{\"id\":\"%s\",\"jx\":%d,\"jy\":%d,\"jw\":%d,\"jh\":%d,\"cx\":%d,\"cy\":%d,\"fsx\":%d,\"fsy\":%d}",
                     firstWidget ? "" : ",",
                     widget.symbolId.c_str(),
                     (int)widget.x, (int)widget.y, (int)widget.width, (int)widget.height,
                     scaledX, scaledY, finalScaleXPct, finalScaleYPct);
            m_debugInfo += wbuf;
            firstWidget = false;

            const bool isTextComponent = (!symbolDef.textLabels.empty()) ||
                                         (symbolDef.rawSvgContent.find("stroke=\"none\"") != std::string::npos &&
                                          symbolDef.rawSvgContent.find("fill=\"none\"") != std::string::npos);

            const float symbolScaleX = m_scaleX * logicalScaleX;
            const float symbolScaleY = m_scaleY * logicalScaleY;
            int32_t symbolStroke = clampStrokePx(widget.strokeWidth);

            if (!isTextComponent)
            {
                m_svgRenderer->renderSymbolFilled(
                    symbol, scaledX, scaledY,
                    fillColor,
                    symbolScaleX, symbolScaleY);
                m_svgRenderer->renderSymbol(
                    symbol, scaledX, scaledY,
                    strokeColor, symbolStroke,
                    symbolScaleX, symbolScaleY, widget.rotation);
            }

            // Render embedded text labels (e.g. "A", "B", "Y")
            for (const auto &lbl : symbolDef.textLabels)
            {
                if (!lbl.text.empty())
                {
                    int32_t lblX = (int32_t)((placeX + 2.0f) * m_scaleX + m_offsetX);
                    int32_t lblY = (int32_t)((placeY + widget.height / 2.0f - 8.0f) * m_scaleY + m_offsetY);
                    int32_t fontSize = (int32_t)(16.0f * m_scaleY);
                    if (fontSize < 14) fontSize = 14;
                    m_canvas->drawText(lblX, lblY, lbl.text.c_str(), LVColor::Black, fontSize);
                }
            }

            if (m_debugMode)
            {
                drawDebugMarker(scaledX, scaledY, widget.symbolId.c_str());

                // --- Green box: widget bounding box as-placed in JSON coords ---
                int32_t wbx = (int32_t)(widget.x * m_scaleX + m_offsetX);
                int32_t wby = (int32_t)(widget.y * m_scaleY + m_offsetY);
                int32_t wbw = (int32_t)(widget.width * m_scaleX);
                int32_t wbh = (int32_t)(widget.height * m_scaleY);
                LVColor green(0, 220, 0);
                m_canvas->drawLine(wbx, wby, wbx + wbw, wby, green, 1);
                m_canvas->drawLine(wbx + wbw, wby, wbx + wbw, wby + wbh, green, 1);
                m_canvas->drawLine(wbx + wbw, wby + wbh, wbx, wby + wbh, green, 1);
                m_canvas->drawLine(wbx, wby + wbh, wbx, wby, green, 1);

                // --- Blue box: viewBox boundary at rendered origin/scale ---
                int32_t vbx = scaledX;
                int32_t vby = scaledY;
                int32_t vbw = (int32_t)(symbol.viewBox.width * symbolScaleX);
                int32_t vbh = (int32_t)(symbol.viewBox.height * symbolScaleY);
                LVColor blue(0, 100, 255);
                m_canvas->drawLine(vbx, vby, vbx + vbw, vby, blue, 1);
                m_canvas->drawLine(vbx + vbw, vby, vbx + vbw, vby + vbh, blue, 1);
                m_canvas->drawLine(vbx + vbw, vby + vbh, vbx, vby + vbh, blue, 1);
                m_canvas->drawLine(vbx, vby + vbh, vbx, vby, blue, 1);

                // --- Cyan centerline: viewBox center Y ---
                int32_t centerY = vby + vbh / 2;
                m_canvas->drawLine(vbx, centerY, vbx + vbw, centerY, LVColor(0, 220, 220), 1);

                // --- Magenta box: actual path bounds inside viewBox (shows intrinsic symbol padding) ---
                if (symbol.pathData)
                {
                    SvgRenderer::SvgPathParser debugParser;
                    const auto debugCommands = debugParser.parse(symbol.pathData);
                    float pMinX = std::numeric_limits<float>::infinity();
                    float pMinY = std::numeric_limits<float>::infinity();
                    float pMaxX = -std::numeric_limits<float>::infinity();
                    float pMaxY = -std::numeric_limits<float>::infinity();
                    float pCx = 0.0f;
                    float pCy = 0.0f;

                    auto includePoint = [&](float px, float py)
                    {
                        if (px < pMinX)
                            pMinX = px;
                        if (py < pMinY)
                            pMinY = py;
                        if (px > pMaxX)
                            pMaxX = px;
                        if (py > pMaxY)
                            pMaxY = py;
                    };

                    for (const auto &cmd : debugCommands)
                    {
                        switch (cmd.type)
                        {
                        case 'M':
                        case 'L':
                            if (cmd.args.size() >= 2)
                            {
                                pCx = cmd.args[0];
                                pCy = cmd.args[1];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'm':
                        case 'l':
                            if (cmd.args.size() >= 2)
                            {
                                pCx += cmd.args[0];
                                pCy += cmd.args[1];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'H':
                            if (cmd.args.size() >= 1)
                            {
                                pCx = cmd.args[0];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'h':
                            if (cmd.args.size() >= 1)
                            {
                                pCx += cmd.args[0];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'V':
                            if (cmd.args.size() >= 1)
                            {
                                pCy = cmd.args[0];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'v':
                            if (cmd.args.size() >= 1)
                            {
                                pCy += cmd.args[0];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'C':
                            if (cmd.args.size() >= 6)
                            {
                                includePoint(cmd.args[0], cmd.args[1]);
                                includePoint(cmd.args[2], cmd.args[3]);
                                pCx = cmd.args[4];
                                pCy = cmd.args[5];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'c':
                            if (cmd.args.size() >= 6)
                            {
                                includePoint(pCx + cmd.args[0], pCy + cmd.args[1]);
                                includePoint(pCx + cmd.args[2], pCy + cmd.args[3]);
                                pCx += cmd.args[4];
                                pCy += cmd.args[5];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'Q':
                            if (cmd.args.size() >= 4)
                            {
                                includePoint(cmd.args[0], cmd.args[1]);
                                pCx = cmd.args[2];
                                pCy = cmd.args[3];
                                includePoint(pCx, pCy);
                            }
                            break;
                        case 'q':
                            if (cmd.args.size() >= 4)
                            {
                                includePoint(pCx + cmd.args[0], pCy + cmd.args[1]);
                                pCx += cmd.args[2];
                                pCy += cmd.args[3];
                                includePoint(pCx, pCy);
                            }
                            break;
                        default:
                            break;
                        }
                    }

                    if (std::isfinite(pMinX) && std::isfinite(pMinY) && std::isfinite(pMaxX) && std::isfinite(pMaxY) &&
                        (pMaxX > pMinX) && (pMaxY > pMinY))
                    {
                        float relMinX = pMinX >= widgetBaseX ? (pMinX - widgetBaseX) : pMinX;
                        float relMaxX = pMinX >= widgetBaseX ? (pMaxX - widgetBaseX) : pMaxX;
                        float relMinY = pMinY >= widgetBaseY ? (pMinY - widgetBaseY) : pMinY;
                        float relMaxY = pMinY >= widgetBaseY ? (pMaxY - widgetBaseY) : pMaxY;

                        int32_t pbx1 = (int32_t)((placeX + relMinX * logicalScaleX) * m_scaleX + m_offsetX);
                        int32_t pby1 = (int32_t)((placeY + relMinY * logicalScaleY) * m_scaleY + m_offsetY);
                        int32_t pbx2 = (int32_t)((placeX + relMaxX * logicalScaleX) * m_scaleX + m_offsetX);
                        int32_t pby2 = (int32_t)((placeY + relMaxY * logicalScaleY) * m_scaleY + m_offsetY);
                        LVColor magenta(255, 0, 255);
                        m_canvas->drawLine(pbx1, pby1, pbx2, pby1, magenta, 1);
                        m_canvas->drawLine(pbx2, pby1, pbx2, pby2, magenta, 1);
                        m_canvas->drawLine(pbx2, pby2, pbx1, pby2, magenta, 1);
                        m_canvas->drawLine(pbx1, pby2, pbx1, pby1, magenta, 1);

                        // Output reference point of symbol path (right-most point at path vertical center)
                        const float outRefXJson = placeX + relMaxX * logicalScaleX;
                        const float outRefYJson = placeY + ((relMinY + relMaxY) * 0.5f) * logicalScaleY;
                        int32_t outRefX = (int32_t)(outRefXJson * m_scaleX + m_offsetX);
                        int32_t outRefY = (int32_t)(outRefYJson * m_scaleY + m_offsetY);
                        m_canvas->drawCircle(outRefX, outRefY, 4, LVColor(50, 230, 50));
                        m_canvas->drawText(outRefX + 6, outRefY - 10, "GOUT", LVColor(20, 160, 20), 12);

                        // Nearest output port alignment check against symbol output reference
                        bool foundOutputPort = false;
                        float bestDist2 = std::numeric_limits<float>::infinity();
                        float portX = 0.0f;
                        float portY = 0.0f;
                        std::string portId;
                        for (const auto &port : screen.ports)
                        {
                            if (port.type != "output")
                                continue;
                            const float dx = port.x - outRefXJson;
                            const float dy = port.y - outRefYJson;
                            const float d2 = dx * dx + dy * dy;
                            if (d2 < bestDist2)
                            {
                                bestDist2 = d2;
                                foundOutputPort = true;
                                portX = port.x;
                                portY = port.y;
                                portId = port.id;
                            }
                        }

                        if (foundOutputPort)
                        {
                            int32_t portCx = (int32_t)(portX * m_scaleX + m_offsetX);
                            int32_t portCy = (int32_t)(portY * m_scaleY + m_offsetY);
                            m_canvas->drawLine(outRefX, outRefY, portCx, portCy, LVColor(255, 210, 0), 1);

                            char abuf[64];
                            snprintf(abuf, sizeof(abuf), "->P%s dx=%d dy=%d",
                                     portId.c_str(),
                                     (int)(portX - outRefXJson),
                                     (int)(portY - outRefYJson));
                            m_canvas->drawText((outRefX + portCx) / 2 + 6, (outRefY + portCy) / 2 - 12,
                                               abuf, LVColor(180, 130, 0), 12);
                        }
                    }
                }
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

        SvgRenderer::SvgPathParser parser;

        // Collect all port and widget pin Y coordinates for exact horizontal snapping
        std::vector<float> pinYCoords;
        for (const auto &p : screen.ports)
        {
            pinYCoords.push_back(p.y);
        }
        for (const auto &w : screen.widgets)
        {
            if (w.type == "svgSymbol" && w.height > 0)
            {
                pinYCoords.push_back(w.y + 15.0f);
                pinYCoords.push_back(w.y + w.height / 2.0f);
                pinYCoords.push_back(w.y + w.height - 15.0f);
            }
        }

        auto snapY = [&](float yVal) -> float {
            for (float targetY : pinYCoords)
            {
                if (std::abs(yVal - targetY) <= 2.5f)
                {
                    return targetY;
                }
            }
            return yVal;
        };

        for (const auto &wire : screen.wires)
        {
            if (wire.path.empty())
            {
                ESP_LOGD(TAG, "  Wire %s: empty path, skipping", wire.id.c_str());
                continue;
            }

            // In debug mode draw wires in red so they are visually distinct from gate outlines.
            SvgRenderer::Color color = m_debugMode
                                           ? SvgRenderer::Color{220, 0, 0}
                                           : parseColor(wire.color.empty() ? "#2C3E50" : wire.color);
            LVColor lvColor(color.r, color.g, color.b);
            int32_t sw = clampStrokePx(wire.strokeWidth);

            auto commands = parser.parse(wire.path.c_str());

            float cx = 0, cy = 0; // current pen position (JSON coords)
            float sx = 0, sy = 0; // subpath start (for Z close)
            bool hasStart = false;
            float startX = 0.0f, startY = 0.0f;
            float endX = 0.0f, endY = 0.0f;

            for (const auto &cmd : commands)
            {
                switch (cmd.type)
                {
                case 'M':
                    cx = cmd.args[0];
                    cy = snapY(cmd.args[1]);
                    sx = cx;
                    sy = cy;
                    if (!hasStart)
                    {
                        hasStart = true;
                        startX = cx;
                        startY = cy;
                    }
                    endX = cx;
                    endY = cy;
                    break;
                case 'm':
                    cx += cmd.args[0];
                    cy += cmd.args[1];
                    cy = snapY(cy);
                    sx = cx;
                    sy = cy;
                    if (!hasStart)
                    {
                        hasStart = true;
                        startX = cx;
                        startY = cy;
                    }
                    endX = cx;
                    endY = cy;
                    break;
                case 'L':
                {
                    float tx = cmd.args[0], ty = snapY(cmd.args[1]);
                    if (std::abs(ty - cy) <= 2.5f)
                    {
                        ty = cy; // Snap nearly horizontal wire to perfectly flat line
                    }
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(tx * m_scaleX + m_offsetX), (int32_t)(ty * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cx = tx;
                    cy = ty;
                    endX = cx;
                    endY = cy;
                    break;
                }
                case 'l':
                {
                    float tx = cx + cmd.args[0], ty = snapY(cy + cmd.args[1]);
                    if (std::abs(ty - cy) <= 2.5f)
                    {
                        ty = cy; // Snap nearly horizontal wire to perfectly flat line
                    }
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(tx * m_scaleX + m_offsetX), (int32_t)(ty * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cx = tx;
                    cy = ty;
                    endX = cx;
                    endY = cy;
                    break;
                }
                case 'H':
                {
                    float tx = cmd.args[0];
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(tx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cx = tx;
                    endX = cx;
                    endY = cy;
                    break;
                }
                case 'h':
                {
                    float tx = cx + cmd.args[0];
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(tx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cx = tx;
                    endX = cx;
                    endY = cy;
                    break;
                }
                case 'V':
                {
                    float ty = cmd.args[0];
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(ty * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cy = ty;
                    endX = cx;
                    endY = cy;
                    break;
                }
                case 'v':
                {
                    float ty = cy + cmd.args[0];
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(ty * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cy = ty;
                    endX = cx;
                    endY = cy;
                    break;
                }
                case 'Z':
                case 'z':
                    m_canvas->drawLine(
                        (int32_t)(cx * m_scaleX + m_offsetX), (int32_t)(cy * m_scaleY + m_offsetY),
                        (int32_t)(sx * m_scaleX + m_offsetX), (int32_t)(sy * m_scaleY + m_offsetY),
                        lvColor, sw);
                    cx = sx;
                    cy = sy;
                    endX = cx;
                    endY = cy;
                    break;
                default:
                    ESP_LOGD(TAG, "  Wire %s: unhandled cmd '%c', skipping segment", wire.id.c_str(), cmd.type);
                    break;
                }
            }

            if (m_debugMode && hasStart)
            {
                int32_t sxp = (int32_t)(startX * m_scaleX + m_offsetX);
                int32_t syp = (int32_t)(startY * m_scaleY + m_offsetY);
                int32_t exp = (int32_t)(endX * m_scaleX + m_offsetX);
                int32_t eyp = (int32_t)(endY * m_scaleY + m_offsetY);

                // Cyan = wire start, Orange = wire end
                m_canvas->drawCircle(sxp, syp, 4, LVColor(0, 255, 255));
                m_canvas->drawCircle(exp, eyp, 4, LVColor(255, 140, 0));

                char sbuf[32];
                snprintf(sbuf, sizeof(sbuf), "W%s:S", wire.id.c_str());
                m_canvas->drawText(sxp + 6, syp - 10, sbuf, LVColor(0, 160, 160), 12);

                char ebuf[32];
                snprintf(ebuf, sizeof(ebuf), "W%s:E", wire.id.c_str());
                m_canvas->drawText(exp + 6, eyp - 10, ebuf, LVColor(180, 90, 0), 12);

                // Flag malformed exports: wire endpoints outside JSON screen bounds.
                const bool startOffscreen = (startX < 0.0f || startY < 0.0f || startX > screen.width || startY > screen.height);
                const bool endOffscreen = (endX < 0.0f || endY < 0.0f || endX > screen.width || endY > screen.height);
                if (startOffscreen || endOffscreen)
                {
                    ESP_LOGW(TAG, "Wire %s endpoint off-screen: S(%.1f,%.1f) E(%.1f,%.1f)",
                             wire.id.c_str(), startX, startY, endX, endY);

                    if (startOffscreen)
                    {
                        m_canvas->drawLine(sxp - 6, syp - 6, sxp + 6, syp + 6, LVColor(255, 0, 0), 2);
                        m_canvas->drawLine(sxp - 6, syp + 6, sxp + 6, syp - 6, LVColor(255, 0, 0), 2);
                    }
                    if (endOffscreen)
                    {
                        m_canvas->drawLine(exp - 6, eyp - 6, exp + 6, eyp + 6, LVColor(255, 0, 0), 2);
                        m_canvas->drawLine(exp - 6, eyp + 6, exp + 6, eyp - 6, LVColor(255, 0, 0), 2);
                    }
                }
            }

            ESP_LOGD(TAG, "  Wire %s: rendered %d commands", wire.id.c_str(), (int)commands.size());
        }
    }

    void JsonRenderer::renderPorts(const Screen &screen)
    {
        if (screen.ports.empty())
            return;

        ESP_LOGI(TAG, "Rendering %d ports...", screen.ports.size());

        for (const auto &port : screen.ports)
        {
            int32_t scaledX = (int32_t)(port.x * m_scaleX + m_offsetX);
            int32_t scaledY = (int32_t)(port.y * m_scaleY + m_offsetY);
            int32_t scaledRadius = (int32_t)(4.5f * m_scaleX);
            if (scaledRadius < 4) scaledRadius = 4;

            auto pointOnSegment = [](float px, float py, float x1, float y1, float x2, float y2) -> bool {
                float minX = std::min(x1, x2) - 2.5f, maxX = std::max(x1, x2) + 2.5f;
                float minY = std::min(y1, y2) - 2.5f, maxY = std::max(y1, y2) + 2.5f;
                if (px < minX || px > maxX || py < minY || py > maxY) return false;
                
                float dx = x2 - x1, dy = y2 - y1;
                float lenSq = dx * dx + dy * dy;
                if (lenSq < 1e-4f) return (std::abs(px - x1) <= 3.5f && std::abs(py - y1) <= 3.5f);
                
                float t = std::max(0.0f, std::min(1.0f, ((px - x1) * dx + (py - y1) * dy) / lenSq));
                float projX = x1 + t * dx, projY = y1 + t * dy;
                float distSq = (px - projX) * (px - projX) + (py - projY) * (py - projY);
                return distSq <= 12.25f;
            };

            int touchCount = 0;
            SvgRenderer::SvgPathParser parser;
            for (const auto &w : screen.wires)
            {
                auto cmds = parser.parse(w.path.c_str());
                float cx = 0, cy = 0;
                for (const auto &cmd : cmds)
                {
                    if (cmd.type == 'M' || cmd.type == 'm')
                    {
                        cx = cmd.args[0]; cy = cmd.args[1];
                    }
                    else if (cmd.type == 'L' || cmd.type == 'l')
                    {
                        float tx = cmd.args[0], ty = cmd.args[1];
                        if (pointOnSegment(port.x, port.y, cx, cy, tx, ty))
                        {
                            touchCount++;
                        }
                        cx = tx; cy = ty;
                    }
                }
            }

            // Auto-detect Junction vs External Port:
            // If 2 or more wire connections touch or branch from this point, it is a Junction!
            if (touchCount >= 2 || port.type == "junction")
            {
                int32_t jRadius = (int32_t)(3.5f * m_scaleX);
                if (jRadius < 3) jRadius = 3;
                m_canvas->drawCircle(scaledX, scaledY, jRadius, LVColor(44, 62, 80), true);
                continue;
            }

            // Configurable hardware port connector colors:
            // Input ports: Solid Blue (#3498DB)
            // Output ports: Solid Yellow (#F1C40F)
            LVColor fillColor;
            if (!port.color.empty() && port.color != "#000000" && port.color != "#000" && port.color != "#2C3E50")
            {
                SvgRenderer::Color c = parseColor(port.color);
                fillColor = LVColor(c.r, c.g, c.b);
            }
            else if (port.type == "input" || port.type == "in")
            {
                fillColor = LVColor(52, 152, 219); // Blue hardware connector (#3498DB)
            }
            else if (port.type == "output" || port.type == "out")
            {
                fillColor = LVColor(241, 196, 15); // Yellow hardware connector (#F1C40F)
            }
            else
            {
                bool isInput = (port.x < screen.width * 0.45f);
                if (isInput)
                {
                    fillColor = LVColor(52, 152, 219); // Blue hardware connector (#3498DB)
                }
                else
                {
                    fillColor = LVColor(241, 196, 15); // Yellow hardware connector (#F1C40F)
                }
            }

            // Draw solid filled circle using LVGL native layer command (masks out wire line underneath)
            m_canvas->drawCircle(scaledX, scaledY, scaledRadius, fillColor, true);
            m_canvas->drawCircle(scaledX, scaledY, scaledRadius, LVColor(44, 62, 80), false);

            if (m_debugMode)
            {
                // Port cross + ID label (quick visual map between JSON and render)
                m_canvas->drawLine(scaledX - 6, scaledY, scaledX + 6, scaledY, LVColor(255, 255, 0), 1);
                m_canvas->drawLine(scaledX, scaledY - 6, scaledX, scaledY + 6, LVColor(255, 255, 0), 1);

                char pbuf[32];
                snprintf(pbuf, sizeof(pbuf), "P%s", port.id.c_str());
                m_canvas->drawText(scaledX + 8, scaledY - 8, pbuf, LVColor(120, 120, 255), 12);
            }

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
            // Draw solid black filled circle for junction
            LVColor color = LVColor(44, 62, 80);

            int32_t scaledX = (int32_t)(junction.x * m_scaleX + m_offsetX);
            int32_t scaledY = (int32_t)(junction.y * m_scaleY + m_offsetY);
            int32_t scaledRadius = (int32_t)(3.5f * m_scaleX);
            if (scaledRadius < 3) scaledRadius = 3;

            m_canvas->drawCircle(
                scaledX,
                scaledY,
                scaledRadius,
                color,
                true);

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
        int32_t canvasW = m_canvas->width();
        int32_t canvasH = m_canvas->height();

        // --- Step 1: content bounding box (widgets + ports + junctions) ---
        float bbMinX = std::numeric_limits<float>::max();
        float bbMinY = std::numeric_limits<float>::max();
        float bbMaxX = std::numeric_limits<float>::lowest();
        float bbMaxY = std::numeric_limits<float>::lowest();
        float maxGateDim = 0.0f; // largest svgSymbol dimension for gate size cap

        for (const auto &w : screen.widgets)
        {
            if (w.width > 0 && w.height > 0)
            {
                bbMinX = std::min(bbMinX, w.x);
                bbMinY = std::min(bbMinY, w.y);
                bbMaxX = std::max(bbMaxX, w.x + w.width);
                bbMaxY = std::max(bbMaxY, w.y + w.height);
                if (w.type == "svgSymbol")
                    maxGateDim = std::max(maxGateDim, std::max(w.width, w.height));
            }
        }
        for (const auto &p : screen.ports)
        {
            bbMinX = std::min(bbMinX, p.x - p.radius);
            bbMinY = std::min(bbMinY, p.y - p.radius);
            bbMaxX = std::max(bbMaxX, p.x + p.radius);
            bbMaxY = std::max(bbMaxY, p.y + p.radius);
        }
        for (const auto &j : screen.junctions)
        {
            bbMinX = std::min(bbMinX, j.x - j.radius);
            bbMinY = std::min(bbMinY, j.y - j.radius);
            bbMaxX = std::max(bbMaxX, j.x + j.radius);
            bbMaxY = std::max(bbMaxY, j.y + j.radius);
        }

        // Fallback to full canvas if no content found
        if (bbMinX >= bbMaxX || bbMinY >= bbMaxY)
        {
            bbMinX = 0;
            bbMinY = 0;
            bbMaxX = (float)screen.width;
            bbMaxY = (float)screen.height;
        }

        // --- Step 2: scale to fit content bbox inside canvas with fixed pixel padding ---
        const float kPadPx = 24.0f; // padding in screen pixels on each side
        float availW = (float)canvasW - 2.0f * kPadPx;
        float availH = (float)canvasH - 2.0f * kPadPx;
        float bbW = bbMaxX - bbMinX;
        float bbH = bbMaxY - bbMinY;
        float scale = std::min(availW / bbW, availH / bbH);

        // --- Step 3: gate size cap — single gate must not dominate the screen ---
        // Ensures single-gate circuits look similar in size to multi-gate circuits.
        if (maxGateDim > 0.0f)
        {
            float scaleCap = kMaxGateScreenPx / maxGateDim;
            scale = std::min(scale, scaleCap);
        }

        m_scaleX = scale;
        m_scaleY = scale;

        // --- Step 4: center the content bounding box in the canvas ---
        float bbCenterX = (bbMinX + bbMaxX) / 2.0f;
        float bbCenterY = (bbMinY + bbMaxY) / 2.0f;
        m_offsetX = canvasW / 2.0f - bbCenterX * scale;
        m_offsetY = canvasH / 2.0f - bbCenterY * scale;

        ESP_LOGI(TAG, "ContentFit: bb=(%.0f,%.0f)-(%.0f,%.0f) gateDim=%.0f scale=%.3f offset=(%.1f,%.1f)",
                 bbMinX, bbMinY, bbMaxX, bbMaxY, maxGateDim, scale, m_offsetX, m_offsetY);
    }

} // namespace JsonRenderer
