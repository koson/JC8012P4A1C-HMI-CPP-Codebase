#pragma once

#include "JsonTypes.hpp"
#include "JsonParser.hpp"
#include "SvgRenderer.hpp"
#include "DrawTarget.hpp"
#include <memory>

namespace JsonRenderer
{

    /**
     * @brief JSON Renderer - Renders complete circuit from JSON file
     *
     * Loads JSON from SD card and renders all elements:
     * - Embedded symbols (v1.1)
     * - Widgets (gates, symbols)
     * - Wires
     * - Ports
     * - Junctions
     *
     * Example:
     *   JsonRenderer renderer(canvas);
     *   if (renderer.loadAndRender("/sdcard/worksheets/half_adder.json")) {
     *       // Circuit rendered successfully
     *   }
     */
    class JsonRenderer
    {
    public:
        /**
         * @brief Constructor
         * @param canvas LVGL canvas to render to
         */
        JsonRenderer(DrawTarget *canvas);
        ~JsonRenderer();

        /**
         * @brief Load JSON file and render to canvas
         * @param filePath Path to JSON file
         * @return true on success, false on error
         */
        bool loadAndRender(const char *filePath);

        /**
         * @brief Render pre-parsed screen
         * @param screen Screen structure
         * @return true on success, false on error
         */
        bool render(const Screen &screen);

        /**
         * @brief Clear canvas
         */
        void clear();

        /**
         * @brief Get last error message
         */
        const char *getLastError() const { return m_lastError.c_str(); }

        /**
         * @brief Get loaded screen info
         */
        const Screen *getScreen() const { return m_screen.get(); }

        /**
         * @brief Enable/disable debug overlay (draws dots at widget anchors)
         */
        void setDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Get debug info string (last render coordinates)
         */
        const std::string &getDebugInfo() const { return m_debugInfo; }

    private:
        // Rendering methods
        void renderWidgets(const Screen &screen);
        void renderWires(const Screen &screen);
        void drawDebugMarker(int32_t x, int32_t y, const char *label);
        void renderPorts(const Screen &screen);
        void renderJunctions(const Screen &screen);

        // Helper methods
        SvgRenderer::Color parseColor(const std::string &hexColor);
        void calculateScale(const Screen &screen);

        DrawTarget *m_canvas;
        std::unique_ptr<SvgRenderer::SvgRenderer> m_svgRenderer;
        std::unique_ptr<JsonParser> m_parser;
        std::unique_ptr<Screen> m_screen;

        // Auto-scaling
        float m_scaleX;
        float m_scaleY;
        float m_offsetX;
        float m_offsetY;
        std::string m_lastError;
        bool m_debugMode = false;
        std::string m_debugInfo;
    };

} // namespace JsonRenderer
