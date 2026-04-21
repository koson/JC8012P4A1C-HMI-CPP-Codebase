#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "SystemManager.h"

// Forward declarations
class VPIHMIUI;
class VPIDocument;
class DataLogger;

/**
 * @brief VPIApplication - VPI HMI application manager
 * 
 * This class manages VPI-specific application logic:
 * - VPI HMI UI creation and management
 * - UART receiver for data input
 * - VPI data manager
 * - CSV data logger
 * - Event handlers
 * 
 * This serves as an example of how to structure an application
 * on top of the SystemManager base.
 */
class VPIApplication {
public:
    /**
     * @brief Get VPIApplication singleton instance
     */
    static VPIApplication& getInstance();

    /**
     * @brief Initialize the VPI HMI application
     * @param sysMgr Reference to SystemManager
     * @return ESP_OK on success
     */
    esp_err_t init(SystemManager& sysMgr);

    /**
     * @brief Start the application (begins data processing)
     * @return ESP_OK on success
     */
    esp_err_t start();

    /**
     * @brief Stop the application
     */
    void stop();

    /**
     * @brief Update electrical data (called from C wrapper)
     */
    void updateElectrical(uint8_t phase, float voltage, float current,
                         float power, float reactive, float energy);

    /**
     * @brief Update pressure data (called from C wrapper)
     */
    void updatePressure(float percentage);

    /**
     * @brief Update status text (called from C wrapper)
     */
    void updateStatus(const char* status);

    /**
     * @brief Get VPI HMI UI instance
     */
    VPIHMIUI* getUI() const { return m_ui; }

    /**
     * @brief Get VPI Document instance
     */
    VPIDocument* getDocument() const { return m_document; }

    /**
     * @brief Get Data Logger instance
     */
    DataLogger* getDataLogger() const { return m_logger; }

private:
    VPIApplication();
    ~VPIApplication();
    VPIApplication(const VPIApplication&) = delete;
    VPIApplication& operator=(const VPIApplication&) = delete;

    /**
     * @brief Initialize VPI-specific components
     */
    esp_err_t initVPIComponents();

    /**
     * @brief Initialize data logger
     */
    esp_err_t initDataLogger(SystemManager& sysMgr);

    /**
     * @brief Setup UI event handlers
     */
    void setupEventHandlers();

    /**
     * @brief Setup timers (datetime update, etc.)
     */
    void setupTimers();

    // Static event callbacks (must be static for C API)
    static void onLogButtonClick(lv_event_t* e);
    static void onDateTimeTimerTick(lv_timer_t* timer);
    static void onGlobalTouch(lv_event_t* e);

    SystemManager* m_sysMgr;
    VPIHMIUI* m_ui;
    VPIDocument* m_document;
    DataLogger* m_logger;
    lv_timer_t* m_datetime_timer;
    bool m_initialized;
    bool m_running;
};
