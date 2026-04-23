#pragma once

#include "JsonTypes.hpp"
#include "cJSON.h"
#include <string>

namespace JsonRenderer
{

    /**
     * @brief JSON Parser for LabBuddy circuit files
     *
     * Parses JSON schema v1.1 with embedded symbols support.
     *
     * Example:
     *   JsonParser parser;
     *   Screen screen;
     *   if (parser.parseFile("/sdcard/worksheets/circuit.json", screen)) {
     *       // Use screen data
     *   }
     */
    class JsonParser
    {
    public:
        JsonParser();
        ~JsonParser();

        /**
         * @brief Parse JSON file from SD card
         * @param filePath Path to JSON file (e.g., "/sdcard/worksheets/circuit.json")
         * @param screen Output screen structure
         * @return true on success, false on error
         */
        bool parseFile(const char *filePath, Screen &screen);

        /**
         * @brief Parse JSON string
         * @param jsonString JSON content
         * @param screen Output screen structure
         * @return true on success, false on error
         */
        bool parseString(const char *jsonString, Screen &screen);

        /**
         * @brief Get last error message
         */
        const char *getLastError() const { return m_lastError.c_str(); }

    private:
        // Parse methods for each section
        bool parseScreen(cJSON *root, Screen &screen);
        bool parseEmbeddedSymbols(cJSON *symbolsObj, Screen &screen);
        bool parseWidgets(cJSON *widgetsArray, Screen &screen);
        bool parseWires(cJSON *wiresArray, Screen &screen);
        bool parsePorts(cJSON *portsArray, Screen &screen);
        bool parseJunctions(cJSON *junctionsArray, Screen &screen);

        // Helper methods
        std::string getString(cJSON *obj, const char *key, const char *defaultValue = "");
        int getInt(cJSON *obj, const char *key, int defaultValue = 0);
        float getFloat(cJSON *obj, const char *key, float defaultValue = 0.0f);
        bool getBool(cJSON *obj, const char *key, bool defaultValue = false);

        std::string m_lastError;
    };

} // namespace JsonRenderer
