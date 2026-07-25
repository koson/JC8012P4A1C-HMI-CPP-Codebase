#include "JsonParser.hpp"
#include "SvgPathParser.hpp"
#include "esp_log.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cctype>
#include <limits>
#include <cmath>

static const char *TAG = "JsonParser";

namespace
{
    static std::string camelToSnake(const char *key)
    {
        if (!key)
        {
            return "";
        }

        std::string snake;
        snake.reserve(strlen(key) + 8);

        for (size_t i = 0; key[i] != '\0'; ++i)
        {
            const unsigned char ch = static_cast<unsigned char>(key[i]);
            if (std::isupper(ch))
            {
                if (i > 0)
                {
                    snake.push_back('_');
                }
                snake.push_back(static_cast<char>(std::tolower(ch)));
            }
            else
            {
                snake.push_back(static_cast<char>(ch));
            }
        }

        return snake;
    }

    static cJSON *getObjectItemFlexible(cJSON *obj, const char *key)
    {
        if (!obj || !key)
        {
            return nullptr;
        }

        // cJSON_GetObjectItem is case-insensitive; first try the original key.
        cJSON *item = cJSON_GetObjectItem(obj, key);
        if (item)
        {
            return item;
        }

        // Fallback for snake_case JSON from WASM pipeline.
        std::string snakeKey = camelToSnake(key);
        if (!snakeKey.empty() && snakeKey != key)
        {
            item = cJSON_GetObjectItem(obj, snakeKey.c_str());
            if (item)
            {
                return item;
            }
        }

        return nullptr;
    }
}

namespace JsonRenderer
{

    JsonParser::JsonParser()
    {
    }

    JsonParser::~JsonParser()
    {
    }

    bool JsonParser::parseFile(const char *filePath, Screen &screen)
    {
        ESP_LOGI(TAG, "Parsing JSON file: %s", filePath);

        // Read file content
        FILE *file = fopen(filePath, "r");
        if (!file)
        {
            m_lastError = "Failed to open file: ";
            m_lastError += filePath;
            ESP_LOGE(TAG, "%s", m_lastError.c_str());
            return false;
        }

        // Get file size
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Read content
        char *buffer = (char *)malloc(fileSize + 1);
        if (!buffer)
        {
            fclose(file);
            m_lastError = "Failed to allocate memory for file";
            ESP_LOGE(TAG, "%s", m_lastError.c_str());
            return false;
        }

        size_t bytesRead = fread(buffer, 1, fileSize, file);
        buffer[bytesRead] = '\0';
        fclose(file);

        // Parse JSON string
        bool result = parseString(buffer, screen);
        free(buffer);

        return result;
    }

    bool JsonParser::parseString(const char *jsonString, Screen &screen)
    {
        cJSON *root = cJSON_Parse(jsonString);
        if (!root)
        {
            m_lastError = "Failed to parse JSON";
            const char *error = cJSON_GetErrorPtr();
            if (error)
            {
                m_lastError += ": ";
                m_lastError += error;
            }
            ESP_LOGE(TAG, "%s", m_lastError.c_str());
            return false;
        }

        bool result = parseScreen(root, screen);
        cJSON_Delete(root);

        if (result)
        {
            ESP_LOGI(TAG, "Successfully parsed JSON:");
            ESP_LOGI(TAG, "  - Version: %s", screen.version.c_str());
            ESP_LOGI(TAG, "  - Title: %s", screen.title.c_str());
            ESP_LOGI(TAG, "  - Size: %dx%d", screen.width, screen.height);
            ESP_LOGI(TAG, "  - Embedded symbols: %d", screen.embeddedSymbols.size());
            ESP_LOGI(TAG, "  - Widgets: %d", screen.widgets.size());
            ESP_LOGI(TAG, "  - Wires: %d", screen.wires.size());
            ESP_LOGI(TAG, "  - Ports: %d", screen.ports.size());
            ESP_LOGI(TAG, "  - Junctions: %d", screen.junctions.size());
        }

        return result;
    }

    bool JsonParser::parseJsonObj(cJSON *root, Screen &screen)
    {
        if (!root)
        {
            m_lastError = "Root cJSON object is NULL";
            return false;
        }

        bool result = parseScreen(root, screen);

        if (result)
        {
            ESP_LOGI(TAG, "Successfully parsed JSON Object:");
            ESP_LOGI(TAG, "  - Version: %s", screen.version.c_str());
            ESP_LOGI(TAG, "  - Title: %s", screen.title.c_str());
            ESP_LOGI(TAG, "  - Size: %dx%d", screen.width, screen.height);
            ESP_LOGI(TAG, "  - Embedded symbols: %d", (int)screen.embeddedSymbols.size());
            ESP_LOGI(TAG, "  - Widgets: %d", (int)screen.widgets.size());
            ESP_LOGI(TAG, "  - Wires: %d", (int)screen.wires.size());
            ESP_LOGI(TAG, "  - Ports: %d", (int)screen.ports.size());
            ESP_LOGI(TAG, "  - Junctions: %d", (int)screen.junctions.size());
        }

        return result;
    }


    bool JsonParser::parseScreen(cJSON *root, Screen &screen)
    {
        // Parse basic info
        screen.version = getString(root, "version", "1.0");
        screen.title = getString(root, "title", "Untitled");
        screen.description = getString(root, "description", "");
        screen.width = getInt(root, "width", 1280);
        screen.height = getInt(root, "height", 800);
        screen.backgroundColor = getString(root, "backgroundColor", "#F5F5F5");
        screen.symbolLibrary = getString(root, "symbolLibrary", "");

        // Parse embedded symbols (v1.1)
        cJSON *embeddedSymbols = getObjectItemFlexible(root, "embeddedSymbols");
        if (embeddedSymbols && cJSON_IsObject(embeddedSymbols))
        {
            if (!parseEmbeddedSymbols(embeddedSymbols, screen))
            {
                return false;
            }
        }

        // Parse widgets
        cJSON *widgets = getObjectItemFlexible(root, "widgets");
        if (widgets && cJSON_IsArray(widgets))
        {
            if (!parseWidgets(widgets, screen))
            {
                return false;
            }
        }

        // Parse wires
        cJSON *wires = getObjectItemFlexible(root, "wires");
        if (wires && cJSON_IsArray(wires))
        {
            if (!parseWires(wires, screen))
            {
                return false;
            }
        }

        // Parse ports
        cJSON *ports = getObjectItemFlexible(root, "ports");
        if (ports && cJSON_IsArray(ports))
        {
            if (!parsePorts(ports, screen))
            {
                return false;
            }
        }

        // Parse junctions
        cJSON *junctions = getObjectItemFlexible(root, "junctions");
        if (junctions && cJSON_IsArray(junctions))
        {
            if (!parseJunctions(junctions, screen))
            {
                return false;
            }
        }

        return true;
    }

    bool JsonParser::parseEmbeddedSymbols(cJSON *symbolsObj, Screen &screen)
    {
        cJSON *symbolItem = symbolsObj->child;

        while (symbolItem)
        {
            SymbolDef symbol;
            symbol.id = symbolItem->string; // Key is the symbol ID

            cJSON *obj = symbolItem;
            symbol.title = getString(obj, "title", "");
            symbol.description = getString(obj, "description", "");
            symbol.category = getString(obj, "category", "Custom");
            std::string pathData = getString(obj, "pathData", "");

            // Standard small bubble path string (comma-separated)
            std::string tiny_bubble1 = "M 80,50 c 0,1.380712 -1.119288,2.5 -2.5,2.5 -1.380712,0 -2.5,-1.119288 -2.5,-2.5 0,-1.380712 1.119288,-2.5 2.5,-2.5 1.380712,0 2.5,1.119288 2.5,2.5 z";
            std::string large_bubble1 = "M 85,50 c 0,2.7614 -2.2386,5 -5,5 -2.7614,0 -5,-2.2386 -5,-5 0,-2.7614 2.2386,-5 5,-5 2.7614,0 5,2.2386 5,5 z";

            // Space-separated version
            std::string tiny_bubble2 = "M 80 50 c 0 1.380712 -1.119288 2.5 -2.5 2.5 -1.380712 0 -2.5 -1.119288 -2.5 -2.5 0 -1.380712 1.119288 -2.5 2.5 -2.5 1.380712 0 2.5 1.119288 2.5 2.5 z";
            std::string large_bubble2 = "M 85 50 c 0 2.7614 -2.2386 5 -5 5 -2.7614 0 -5 -2.2386 -5 -5 0 -2.7614 2.2386 -5 5 -5 2.7614 0 5 2.2386 5 5 z";

            size_t pos1 = pathData.find(tiny_bubble1);
            if (pos1 != std::string::npos)
            {
                pathData.replace(pos1, tiny_bubble1.length(), large_bubble1);
                ESP_LOGI(TAG, "Systematically patched tiny bubble (comma-format) for symbol: %s", symbol.id.c_str());
            }

            size_t pos2 = pathData.find(tiny_bubble2);
            if (pos2 != std::string::npos)
            {
                pathData.replace(pos2, tiny_bubble2.length(), large_bubble2);
                ESP_LOGI(TAG, "Systematically patched tiny bubble (space-format) for symbol: %s", symbol.id.c_str());
            }

            std::string rawSvgContent = getString(obj, "rawSvgContent", "");
            if (!rawSvgContent.empty())
            {
                size_t searchPos = 0;
                while ((searchPos = rawSvgContent.find("<ellipse", searchPos)) != std::string::npos)
                {
                    size_t endTag = rawSvgContent.find(">", searchPos);
                    if (endTag == std::string::npos) break;
                    std::string tag = rawSvgContent.substr(searchPos, endTag - searchPos + 1);

                    auto getAttr = [&](const char *attr) -> float {
                        size_t p = tag.find(attr);
                        if (p != std::string::npos) {
                            p = tag.find("\"", p);
                            if (p != std::string::npos) {
                                return std::strtof(tag.c_str() + p + 1, nullptr);
                            }
                        }
                        return 0.0f;
                    };

                    float cx = getAttr("cx=");
                    float cy = getAttr("cy=");
                    float rx = getAttr("rx=");
                    float ry = getAttr("ry=");

                    if (rx > 0 && ry > 0) {
                        if (rx < 4.5f) rx = 5.0f;
                        if (ry < 4.5f) ry = 5.0f;

                        // Dynamically find max X of body polygon near cy
                        float bodyMaxX = -std::numeric_limits<float>::infinity();
                        SvgRenderer::SvgPathParser pathParser;
                        const auto cmds = pathParser.parse(pathData.c_str());
                        float currentX = 0.0f, currentY = 0.0f;
                        for (const auto &cmd : cmds)
                        {
                            bool isRel = (cmd.type >= 'a' && cmd.type <= 'z');
                            for (size_t i = 0; i + 1 < cmd.args.size(); i += 2)
                            {
                                float px = (isRel ? currentX : 0.0f) + cmd.args[i];
                                float py = (isRel ? currentY : 0.0f) + cmd.args[i + 1];
                                if (std::abs(py - cy) < 5.0f && px > bodyMaxX)
                                {
                                    bodyMaxX = px;
                                }
                            }
                            if (!cmd.args.empty() && cmd.args.size() >= 2)
                            {
                                currentX = (isRel ? currentX : 0.0f) + cmd.args[cmd.args.size() - 2];
                                currentY = (isRel ? currentY : 0.0f) + cmd.args[cmd.args.size() - 1];
                            }
                        }

                        // Tangent touch adjustment (dynamic, zero hardcoding):
                        // align bubble left edge with body tip
                        if (std::isfinite(bodyMaxX) && cx - rx < bodyMaxX && cx >= bodyMaxX)
                        {
                            cx = bodyMaxX + rx;
                        }

                        float kx = rx * 0.55228475f;
                        float ky = ry * 0.55228475f;
                        char buf[200];
                        snprintf(buf, sizeof(buf),
                                 " M %.2f %.2f C %.2f %.2f %.2f %.2f %.2f %.2f C %.2f %.2f %.2f %.2f %.2f %.2f C %.2f %.2f %.2f %.2f %.2f %.2f C %.2f %.2f %.2f %.2f %.2f %.2f Z",
                                 cx + rx, cy,
                                 cx + rx, cy + ky, cx + kx, cy + ry, cx, cy + ry,
                                 cx - kx, cy + ry, cx - rx, cy + ky, cx - rx, cy,
                                 cx - rx, cy - ky, cx - kx, cy - ry, cx, cy - ry,
                                 cx + kx, cy - ry, cx + rx, cy - ky, cx + rx, cy);
                        // Trim any pin line segment inside pathData that passes through the extracted ellipse
                        float bubbleOuterX = cx + rx;
                        char searchPin[64];
                        snprintf(searchPin, sizeof(searchPin), "M %.0f %.0f L", cx - 3.75f, cy); // e.g. "M 150 30 L"
                        size_t pinPos = pathData.find("M ");
                        while (pinPos != std::string::npos)
                        {
                            float px1 = 0, py1 = 0, px2 = 0, py2 = 0;
                            int nmatched = sscanf(pathData.c_str() + pinPos, "M %f %f L %f %f", &px1, &py1, &px2, &py2);
                            if (nmatched == 4 && std::abs(py1 - cy) < 1.0f && std::abs(py2 - cy) < 1.0f)
                            {
                                if (px1 < bubbleOuterX && px2 >= bubbleOuterX)
                                {
                                    char origSegment[128], newSegment[128];
                                    snprintf(origSegment, sizeof(origSegment), "M %.2f %.2f L %.2f %.2f", px1, py1, px2, py2);
                                    snprintf(newSegment, sizeof(newSegment), "M %.2f %.2f L %.2f %.2f", bubbleOuterX, py1, px2, py2);

                                    // Match formatted float strings in pathData
                                    size_t segPos = pathData.find(origSegment);
                                    if (segPos == std::string::npos)
                                    {
                                        snprintf(origSegment, sizeof(origSegment), "M %.0f %.0f L %.0f %.0f", px1, py1, px2, py2);
                                        segPos = pathData.find(origSegment);
                                    }
                                    if (segPos != std::string::npos)
                                    {
                                        pathData.replace(segPos, strlen(origSegment), newSegment);
                                        ESP_LOGI(TAG, "Trimmed pin line inside bubble for %s: %s -> %s", symbol.id.c_str(), origSegment, newSegment);
                                    }
                                    break;
                                }
                            }
                            pinPos = pathData.find("M ", pinPos + 2);
                        }

                        pathData += buf;
                        ESP_LOGI(TAG, "Extracted ellipse at cx=%.1f cy=%.1f rx=%.1f ry=%.1f for symbol: %s", cx, cy, rx, ry, symbol.id.c_str());
                    }
                    searchPos = endTag + 1;
                }
            }

            symbol.pathData = pathData;
            symbol.rawSvgContent = rawSvgContent;

            // Parse textLabels
            cJSON *textLabels = getObjectItemFlexible(obj, "textLabels");
            if (textLabels && cJSON_IsArray(textLabels))
            {
                int count = cJSON_GetArraySize(textLabels);
                for (int i = 0; i < count; i++)
                {
                    cJSON *lbl = cJSON_GetArrayItem(textLabels, i);
                    if (lbl && cJSON_IsObject(lbl))
                    {
                        TextLabel tl;
                        tl.text = getString(lbl, "text", "");
                        tl.x = getFloat(lbl, "x", 0.0f);
                        tl.y = getFloat(lbl, "y", 0.0f);
                        if (!tl.text.empty())
                        {
                            symbol.textLabels.push_back(tl);
                        }
                    }
                }
            }

            // Parse viewBox
            cJSON *viewBox = getObjectItemFlexible(obj, "viewBox");
            if (viewBox && cJSON_IsObject(viewBox))
            {
                symbol.viewBox.x = getFloat(viewBox, "X", 0.0f);
                symbol.viewBox.y = getFloat(viewBox, "Y", 0.0f);
                symbol.viewBox.width = getFloat(viewBox, "Width", 150.0f);
                symbol.viewBox.height = getFloat(viewBox, "Height", 150.0f);
            }

            if (symbol.textLabels.empty() && !rawSvgContent.empty())
            {
                size_t textPos = rawSvgContent.find("<text");
                while (textPos != std::string::npos)
                {
                    size_t closeTag = rawSvgContent.find(">", textPos);
                    size_t endTextTag = rawSvgContent.find("</text>", closeTag);
                    if (closeTag != std::string::npos && endTextTag != std::string::npos)
                    {
                        std::string content = rawSvgContent.substr(closeTag + 1, endTextTag - closeTag - 1);
                        size_t first = content.find_first_not_of(" \t\n\r");
                        if (first != std::string::npos)
                        {
                            size_t last = content.find_last_not_of(" \t\n\r");
                            content = content.substr(first, (last - first + 1));
                        }
                        if (!content.empty())
                        {
                            TextLabel tl;
                            tl.text = content;
                            std::string tagHeader = rawSvgContent.substr(textPos, closeTag - textPos);
                            auto getAttr = [&](const char *attr) -> float {
                                size_t p = tagHeader.find(attr);
                                if (p != std::string::npos) {
                                    p = tagHeader.find("\"", p);
                                    if (p != std::string::npos) {
                                        return std::strtof(tagHeader.c_str() + p + 1, nullptr);
                                    }
                                }
                                return 0.0f;
                            };
                            tl.x = getAttr("x=");
                            tl.y = getAttr("y=");
                            symbol.textLabels.push_back(tl);
                        }
                    }
                    textPos = rawSvgContent.find("<text", textPos + 5);
                }
            }

            // Auto-correct viewBox.x and viewBox.y for dynamic symbols where path data starts at minPathX/Y != viewBox.x/y
            if (!pathData.empty())
            {
                SvgRenderer::SvgPathParser pathParser;
                const auto cmds = pathParser.parse(symbol.pathData.c_str());
                float minPathX = 1e9f;
                float minPathY = 1e9f;
                for (const auto &cmd : cmds)
                {
                    if (cmd.type == 'M' || cmd.type == 'L' || cmd.type == 'C' || cmd.type == 'Q')
                    {
                        if (cmd.args.size() >= 2)
                        {
                            if (cmd.args[0] < minPathX) minPathX = cmd.args[0];
                            if (cmd.args[1] < minPathY) minPathY = cmd.args[1];
                        }
                    }
                }
                if (symbol.viewBox.x == 0.0f && minPathX > 0.0f && minPathX < 1e8f)
                {
                    symbol.viewBox.x = minPathX;
                    ESP_LOGI(TAG, "Auto-corrected viewBox.x for dynamic symbol [%s] to %.1f", symbol.id.c_str(), minPathX);
                }
                if (symbol.viewBox.y == 0.0f && std::isfinite(minPathY) && std::abs(minPathY) < 1e5f)
                {
                    symbol.viewBox.y = minPathY;
                    ESP_LOGI(TAG, "Auto-corrected viewBox.y for dynamic symbol [%s] to %.1f", symbol.id.c_str(), minPathY);
                }
            }

            screen.embeddedSymbols[symbol.id] = symbol;

            symbolItem = symbolItem->next;
        }

        return true;
    }

    bool JsonParser::parseWidgets(cJSON *widgetsArray, Screen &screen)
    {
        int count = cJSON_GetArraySize(widgetsArray);

        for (int i = 0; i < count; i++)
        {
            cJSON *item = cJSON_GetArrayItem(widgetsArray, i);

            Widget widget;
            widget.type = getString(item, "type", "svgSymbol");
            widget.symbolId = getString(item, "symbolId", "");
            widget.x = getFloat(item, "x", 0.0f);
            widget.y = getFloat(item, "y", 0.0f);
            widget.scale = getFloat(item, "scale", 1.0f);
            widget.rotation = getFloat(item, "rotation", 0.0f);
            widget.fillColor = getString(item, "fillColor", "#ECF0F1");
            widget.strokeColor = getString(item, "strokeColor", "#2C3E50");
            widget.strokeWidth = getFloat(item, "strokeWidth", 2.0f);
            widget.flipHorizontal = getBool(item, "flipHorizontal", false);
            widget.flipVertical = getBool(item, "flipVertical", false);

            // For type="path": SVG path data
            widget.d = getString(item, "d", "");
            widget.fill = getString(item, "fill", "#000000");
            // "stroke" is the key used by PathWidgetDefinition; fall back to strokeColor if not present
            {
                std::string strokeVal = getString(item, "stroke", "");
                if (!strokeVal.empty() && strokeVal != "none" && widget.strokeColor == "#2C3E50")
                    widget.strokeColor = strokeVal;
            }

            // For type="rect"
            widget.width = getFloat(item, "width", 0.0f);
            widget.height = getFloat(item, "height", 0.0f);

            // For type="circle" — radius is average, rx/ry derived from width/height if present
            widget.radius = getFloat(item, "radius", 5.0f);
            {
                float w = getFloat(item, "width", 0.0f);
                float h = getFloat(item, "height", 0.0f);
                widget.rx = (w > 0) ? w / 2.0f : widget.radius;
                widget.ry = (h > 0) ? h / 2.0f : widget.radius;
            }

            // Additional properties for labels
            widget.text = getString(item, "text", "");
            widget.fontSize = getInt(item, "fontSize", 16);
            widget.textColor = getString(item, "textColor", "#000000");
            widget.fontWeight = getString(item, "fontWeight", "normal");

            screen.widgets.push_back(widget);
        }

        return true;
    }

    bool JsonParser::parseWires(cJSON *wiresArray, Screen &screen)
    {
        int count = cJSON_GetArraySize(wiresArray);

        for (int i = 0; i < count; i++)
        {
            cJSON *item = cJSON_GetArrayItem(wiresArray, i);

            Wire wire;
            wire.id = getString(item, "id", "");
            wire.path = getString(item, "path", "");
            wire.strokeWidth = getFloat(item, "strokeWidth", 2.0f);
            wire.color = getString(item, "color", "#2C3E50");
            wire.routingStyle = getString(item, "routingStyle", "Direct");

            screen.wires.push_back(wire);
        }

        return true;
    }

    bool JsonParser::parsePorts(cJSON *portsArray, Screen &screen)
    {
        int count = cJSON_GetArraySize(portsArray);

        for (int i = 0; i < count; i++)
        {
            cJSON *item = cJSON_GetArrayItem(portsArray, i);

            Port port;
            port.id = getString(item, "id", "");
            port.label = getString(item, "label", "");
            port.type = getString(item, "type", "input");
            port.x = getFloat(item, "x", 0.0f);
            port.y = getFloat(item, "y", 0.0f);
            port.radius = getFloat(item, "radius", 5.0f);
            port.color = getString(item, "color", "#2C3E50");

            screen.ports.push_back(port);
        }

        return true;
    }

    bool JsonParser::parseJunctions(cJSON *junctionsArray, Screen &screen)
    {
        int count = cJSON_GetArraySize(junctionsArray);

        for (int i = 0; i < count; i++)
        {
            cJSON *item = cJSON_GetArrayItem(junctionsArray, i);

            Junction junction;
            junction.id = getString(item, "id", "");
            junction.x = getFloat(item, "x", 0.0f);
            junction.y = getFloat(item, "y", 0.0f);
            junction.radius = getFloat(item, "radius", 3.0f);

            screen.junctions.push_back(junction);
        }

        return true;
    }

    // Helper methods
    std::string JsonParser::getString(cJSON *obj, const char *key, const char *defaultValue)
    {
        cJSON *item = getObjectItemFlexible(obj, key);
        if (item && cJSON_IsString(item))
        {
            return std::string(item->valuestring);
        }
        return std::string(defaultValue);
    }

    int JsonParser::getInt(cJSON *obj, const char *key, int defaultValue)
    {
        cJSON *item = getObjectItemFlexible(obj, key);
        if (item && cJSON_IsNumber(item))
        {
            return item->valueint;
        }
        if (item && cJSON_IsString(item) && item->valuestring)
        {
            char *endPtr = nullptr;
            long parsed = std::strtol(item->valuestring, &endPtr, 10);
            if (endPtr != item->valuestring)
            {
                return (int)parsed;
            }
        }
        return defaultValue;
    }

    float JsonParser::getFloat(cJSON *obj, const char *key, float defaultValue)
    {
        cJSON *item = getObjectItemFlexible(obj, key);
        if (item && cJSON_IsNumber(item))
        {
            return (float)item->valuedouble;
        }
        if (item && cJSON_IsString(item) && item->valuestring)
        {
            char *endPtr = nullptr;
            float parsed = std::strtof(item->valuestring, &endPtr);
            if (endPtr != item->valuestring)
            {
                return parsed;
            }
        }
        return defaultValue;
    }

    bool JsonParser::getBool(cJSON *obj, const char *key, bool defaultValue)
    {
        cJSON *item = getObjectItemFlexible(obj, key);
        if (item && cJSON_IsBool(item))
        {
            return cJSON_IsTrue(item);
        }
        return defaultValue;
    }

} // namespace JsonRenderer
