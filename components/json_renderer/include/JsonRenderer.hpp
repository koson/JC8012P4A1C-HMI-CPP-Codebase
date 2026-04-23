#pragma once

#include "JsonTypes.hpp"
#include "JsonParser.hpp"
#include "SvgRenderer.hpp"
#include "LVCanvas.hpp"
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
        JsonRenderer(LVCanvas *canvas);
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

    private:
        // Rendering methods
        void renderWidgets(const Screen &screen);
        void renderWires(const Screen &screen);
        void renderPorts(const Screen &screen);
        void renderJunctions(const Screen &screen);

        // Helper methods
        SvgRenderer::Color parseColor(const std::string &hexColor);

        LVCanvas *m_canvas;
        std::unique_ptr<SvgRenderer::SvgRenderer> m_svgRenderer;
        std::unique_ptr<JsonParser> m_parser;
        std::unique_ptr<Screen> m_screen;
        std::string m_lastError;
    };

} // namespace JsonRenderer
