#pragma once

#include "esp_err.h"
#include "SystemManager.h"
#include "DemoManager.h"
#include <memory>

/**
 * @brief MenuDemoApplication - Demo gallery menu application
 * 
 * This application demonstrates using SystemManager with a different app:
 * - No UART/VPI components
 * - No data logging
 * - Just a menu to browse and run LVGL demos
 * 
 * This shows how easy it is to create different applications
 * on top of the same SystemManager base.
 */
class MenuDemoApplication {
public:
    /**
     * @brief Get MenuDemoApplication singleton instance
     */
    static MenuDemoApplication& getInstance();

    /**
     * @brief Initialize the menu demo application
     * @param sysMgr Reference to SystemManager
     * @return ESP_OK on success
     */
    esp_err_t init(SystemManager& sysMgr);

    /**
     * @brief Start the application (shows main menu)
     * @return ESP_OK on success
     */
    esp_err_t start();

    /**
     * @brief Stop the application
     */
    void stop();

private:
    MenuDemoApplication();
    ~MenuDemoApplication() = default;
    MenuDemoApplication(const MenuDemoApplication&) = delete;
    MenuDemoApplication& operator=(const MenuDemoApplication&) = delete;

    // Private methods
    void registerAllDemos();

    SystemManager* m_sysMgr;
    std::unique_ptr<DemoManager> m_demoManager;
    bool m_initialized;
    bool m_running;
};
