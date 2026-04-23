#include "JsonParser.hpp"
#include "esp_log.h"
#include <fstream>
#include <sstream>

static const char *TAG = "JsonParser";

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
        cJSON *embeddedSymbols = cJSON_GetObjectItem(root, "embeddedSymbols");
        if (embeddedSymbols && cJSON_IsObject(embeddedSymbols))
        {
            if (!parseEmbeddedSymbols(embeddedSymbols, screen))
            {
                return false;
            }
        }

        // Parse widgets
        cJSON *widgets = cJSON_GetObjectItem(root, "widgets");
        if (widgets && cJSON_IsArray(widgets))
        {
            if (!parseWidgets(widgets, screen))
            {
                return false;
            }
        }

        // Parse wires
        cJSON *wires = cJSON_GetObjectItem(root, "wires");
        if (wires && cJSON_IsArray(wires))
        {
            if (!parseWires(wires, screen))
            {
                return false;
            }
        }

        // Parse ports
        cJSON *ports = cJSON_GetObjectItem(root, "ports");
        if (ports && cJSON_IsArray(ports))
        {
            if (!parsePorts(ports, screen))
            {
                return false;
            }
        }

        // Parse junctions
        cJSON *junctions = cJSON_GetObjectItem(root, "junctions");
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
            symbol.pathData = getString(obj, "pathData", "");

            // Parse viewBox
            cJSON *viewBox = cJSON_GetObjectItem(obj, "viewBox");
            if (viewBox && cJSON_IsObject(viewBox))
            {
                symbol.viewBox.x = getFloat(viewBox, "X", 0.0f);
                symbol.viewBox.y = getFloat(viewBox, "Y", 0.0f);
                symbol.viewBox.width = getFloat(viewBox, "Width", 150.0f);
                symbol.viewBox.height = getFloat(viewBox, "Height", 150.0f);
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

            // Additional properties for labels
            widget.text = getString(item, "text", "");
            widget.fontSize = getInt(item, "fontSize", 16);

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
        cJSON *item = cJSON_GetObjectItem(obj, key);
        if (item && cJSON_IsString(item))
        {
            return std::string(item->valuestring);
        }
        return std::string(defaultValue);
    }

    int JsonParser::getInt(cJSON *obj, const char *key, int defaultValue)
    {
        cJSON *item = cJSON_GetObjectItem(obj, key);
        if (item && cJSON_IsNumber(item))
        {
            return item->valueint;
        }
        return defaultValue;
    }

    float JsonParser::getFloat(cJSON *obj, const char *key, float defaultValue)
    {
        cJSON *item = cJSON_GetObjectItem(obj, key);
        if (item && cJSON_IsNumber(item))
        {
            return (float)item->valuedouble;
        }
        return defaultValue;
    }

    bool JsonParser::getBool(cJSON *obj, const char *key, bool defaultValue)
    {
        cJSON *item = cJSON_GetObjectItem(obj, key);
        if (item && cJSON_IsBool(item))
        {
            return cJSON_IsTrue(item);
        }
        return defaultValue;
    }

} // namespace JsonRenderer
