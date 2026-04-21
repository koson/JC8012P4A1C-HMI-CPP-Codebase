#include "VPIApplication.h"
#include "VPIHMIUI.h"
#include "VPIDocument.h"
#include "DataLogger.h"
#include "VPIDocumentCWrapper.h"
#include "esp_log.h"
#include <time.h>

// C components
extern "C" {
    #include "uart_receiver.h"
    #include "vpi_data_manager.h"
}

static const char* TAG = "VPIApplication";

VPIApplication& VPIApplication::getInstance() {
    static VPIApplication instance;
    return instance;
}

VPIApplication::VPIApplication()
    : m_sysMgr(nullptr)
    , m_ui(nullptr)
    , m_document(nullptr)
    , m_logger(nullptr)
    , m_datetime_timer(nullptr)
    , m_initialized(false)
    , m_running(false)
{
}

VPIApplication::~VPIApplication() {
    stop();
    
    if (m_datetime_timer) {
        lv_timer_del(m_datetime_timer);
        m_datetime_timer = nullptr;
    }
    
    if (m_logger) {
        delete m_logger;
        m_logger = nullptr;
    }
    
    if (m_document) {
        delete m_document;
        m_document = nullptr;
    }
    
    if (m_ui) {
        delete m_ui;
        m_ui = nullptr;
    }
}

esp_err_t VPIApplication::init(SystemManager& sysMgr) {
    if (m_initialized) {
        ESP_LOGW(TAG, "Application already initialized");
        return ESP_OK;
    }

    m_sysMgr = &sysMgr;

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  VPI HMI Application for ESP32-P4");
    ESP_LOGI(TAG, "  Vacuum Pressure Impregnation Monitor");
    ESP_LOGI(TAG, "  C++ OOP + LVGL v9.2.2");
    ESP_LOGI(TAG, "===========================================");

    // Initialize VPI-specific components
    esp_err_t ret = initVPIComponents();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize VPI components");
        return ret;
    }

    // Lock display for UI creation
    if (!m_sysMgr->lockDisplay()) {
        ESP_LOGE(TAG, "Failed to lock display");
        return ESP_FAIL;
    }

    // Create VPI HMI UI
    ESP_LOGI(TAG, "Creating VPI HMI UI (C++ OOP, landscape 1280x800)...");
    m_ui = new VPIHMIUI();
    if (m_ui == nullptr) {
        ESP_LOGE(TAG, "Failed to create UI");
        m_sysMgr->unlockDisplay();
        return ESP_ERR_NO_MEM;
    }

    // Setup event handlers and timers
    setupEventHandlers();
    setupTimers();

    m_sysMgr->unlockDisplay();
    ESP_LOGI(TAG, "✓ VPI HMI UI created");

    // Initialize data logger if SD card is available
    if (m_sysMgr->isSDCardMounted()) {
        ret = initDataLogger(sysMgr);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Data logger initialization failed, continuing without logging");
        }
    } else {
        ESP_LOGW(TAG, "SD card not mounted, data logger disabled");
    }

    m_initialized = true;
    ESP_LOGI(TAG, "✓ VPI HMI Application initialized");
    return ESP_OK;
}

esp_err_t VPIApplication::initVPIComponents() {
    // Initialize UART receiver
    ESP_LOGI(TAG, "Initializing UART receiver...");
    esp_err_t ret = uart_receiver_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART receiver: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize VPI data manager
    ESP_LOGI(TAG, "Initializing VPI data manager...");
    ret = vpi_data_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize VPI data manager: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "✓ VPI components initialized");
    return ESP_OK;
}

esp_err_t VPIApplication::initDataLogger(SystemManager& sysMgr) {
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  Initializing CSV Data Logger");
    ESP_LOGI(TAG, "===========================================");

    // Test SD card write capability
    if (!sysMgr.testSDCardWrite("/sdcard/logs")) {
        ESP_LOGE(TAG, "SD card write test failed");
        ESP_LOGW(TAG, "CSV logging will be disabled");
        return ESP_FAIL;
    }

    // Create VPIDocument (data model with Observer pattern)
    m_document = new VPIDocument();
    if (m_document == nullptr) {
        ESP_LOGE(TAG, "Failed to create VPIDocument");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "✓ VPIDocument created");

    // Create DataLogger with default config
    DataLogger::Config logger_config = DataLogger::getDefaultConfig();
    logger_config.interval_ms = 1000;          // Log every 1 second
    logger_config.max_file_size_mb = 10;       // Rotate at 10 MB
    logger_config.log_on_state_change = true;  // Force log on state change
    logger_config.base_path = "/sdcard/logs";  // Use subdirectory

    m_logger = new DataLogger(logger_config);
    if (m_logger == nullptr) {
        ESP_LOGE(TAG, "Failed to create DataLogger");
        delete m_document;
        m_document = nullptr;
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "✓ DataLogger created (interval=1s, max_size=10MB)");

    // Attach DataLogger as observer to VPIDocument
    m_document->attach(m_logger);
    ESP_LOGI(TAG, "✓ DataLogger attached to VPIDocument");

    // Set VPIDocument handle for vpi_data_manager (C wrapper)
    vpi_document_handle_t doc_handle = static_cast<vpi_document_handle_t>(m_document);
    vpi_data_manager_set_document(doc_handle);
    ESP_LOGI(TAG, "✓ VPIDocument handle set for data manager");

    // Start logging automatically
    if (m_logger->startLogging()) {
        ESP_LOGI(TAG, "✓ CSV logging started: %s", m_logger->getCurrentFilename().c_str());
        
        // Sync button state with logging state
        if (m_ui && m_sysMgr->lockDisplay()) {
            lv_obj_t* log_btn = m_ui->getLogButton();
            if (log_btn) {
                lv_obj_add_state(log_btn, LV_STATE_CHECKED);
                m_ui->setLoggingState(true);
                ESP_LOGI(TAG, "✓ LOG button synced (CHECKED + RED)");
            }
            m_sysMgr->unlockDisplay();
        }
    } else {
        ESP_LOGE(TAG, "✗ Failed to start CSV logging");
    }

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  Data Logger Ready!");
    ESP_LOGI(TAG, "===========================================");

    return ESP_OK;
}

void VPIApplication::setupEventHandlers() {
    if (!m_ui) {
        ESP_LOGW(TAG, "UI not created, skipping event handler setup");
        return;
    }

    // Add global touch event handler to screen for debugging
    lv_obj_t* screen = lv_scr_act();
    lv_obj_add_event_cb(screen, onGlobalTouch, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(screen, onGlobalTouch, LV_EVENT_RELEASED, nullptr);
    ESP_LOGI(TAG, "✓ Global touch event handler registered");

    // Setup LOG button event handler
    lv_obj_t* log_btn = m_ui->getLogButton();
    if (log_btn) {
        lv_obj_add_event_cb(log_btn, onLogButtonClick, LV_EVENT_CLICKED, nullptr);
        ESP_LOGI(TAG, "✓ LOG button event handler registered (btn=%p)", log_btn);

        // Debug: Print button position
        lv_area_t coords;
        lv_obj_get_coords(log_btn, &coords);
        ESP_LOGI(TAG, "  Button absolute: X1=%d, Y1=%d, X2=%d, Y2=%d",
                 coords.x1, coords.y1, coords.x2, coords.y2);
    } else {
        ESP_LOGW(TAG, "LOG button not found");
    }
}

void VPIApplication::setupTimers() {
    // Create DateTime update timer (1000ms = 1 second)
    m_datetime_timer = lv_timer_create(onDateTimeTimerTick, 1000, nullptr);
    if (m_datetime_timer) {
        ESP_LOGI(TAG, "✓ DateTime update timer created (1s interval)");
        // Trigger immediately for initial update
        lv_timer_ready(m_datetime_timer);
    } else {
        ESP_LOGW(TAG, "Failed to create datetime timer");
    }
}

esp_err_t VPIApplication::start() {
    if (!m_initialized) {
        ESP_LOGE(TAG, "Application not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_running) {
        ESP_LOGW(TAG, "Application already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting VPI HMI Application...");

    // Start UART receiver
    ESP_LOGI(TAG, "Starting UART receiver...");
    esp_err_t ret = uart_receiver_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start UART receiver: %s", esp_err_to_name(ret));
        return ret;
    }

    // Start VPI data manager
    ESP_LOGI(TAG, "Starting VPI data manager...");
    ret = vpi_data_manager_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start VPI data manager: %s", esp_err_to_name(ret));
        return ret;
    }

    m_running = true;
    ESP_LOGI(TAG, "✓ VPI HMI Application started");
    ESP_LOGI(TAG, "Waiting for data from Gateway...");

    return ESP_OK;
}

void VPIApplication::stop() {
    if (!m_running) {
        return;
    }

    ESP_LOGI(TAG, "Stopping VPI HMI Application...");

    // Stop data logger
    if (m_logger) {
        m_logger->stopLogging();
    }

    m_running = false;
    ESP_LOGI(TAG, "✓ VPI HMI Application stopped");
}

void VPIApplication::updateElectrical(uint8_t phase, float voltage, float current,
                                     float power, float reactive, float energy) {
    if (m_ui) {
        m_ui->updateElectrical(phase, voltage, current, power, reactive, energy);
    }
}

void VPIApplication::updatePressure(float percentage) {
    ESP_LOGI(TAG, "Update pressure %.2f%%", percentage);
    if (m_ui) {
        m_ui->updatePressure(percentage);
    } else {
        ESP_LOGW(TAG, "UI not initialized");
    }
}

void VPIApplication::updateStatus(const char* status) {
    if (m_ui) {
        m_ui->updateStatus(status);
    }
}

// ========== Static Event Callbacks ==========

void VPIApplication::onLogButtonClick(lv_event_t* e) {
    VPIApplication& app = getInstance();
    lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));

    ESP_LOGI(TAG, "LOG button clicked");

    bool is_checked = lv_obj_has_state(btn, LV_STATE_CHECKED);
    ESP_LOGI(TAG, "Button state: %s", is_checked ? "CHECKED" : "UNCHECKED");

    if (app.m_logger) {
        if (is_checked) {
            // Button now checked = START logging
            ESP_LOGI(TAG, "Starting logging...");
            bool started = app.m_logger->startLogging();

            if (started) {
                app.m_ui->setLoggingState(true);
                ESP_LOGI(TAG, "✓ CSV logging started");
            } else {
                ESP_LOGE(TAG, "✗ Failed to start logging");
                // Revert button state
                lv_obj_remove_state(btn, LV_STATE_CHECKED);
                app.m_ui->setLoggingState(false);
            }
        } else {
            // Button unchecked = STOP logging
            ESP_LOGI(TAG, "Stopping logging...");
            app.m_logger->stopLogging();
            app.m_ui->setLoggingState(false);
            ESP_LOGI(TAG, "✓ CSV logging stopped");
        }
    } else {
        ESP_LOGW(TAG, "DataLogger not initialized");
        // Revert button state
        lv_obj_remove_state(btn, LV_STATE_CHECKED);
    }
}

void VPIApplication::onDateTimeTimerTick(lv_timer_t* timer) {
    VPIApplication& app = getInstance();
    
    if (app.m_ui) {
        // Get current time
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        // Format: "YYYY-MM-DD HH:MM:SS"
        char datetime_str[32];
        strftime(datetime_str, sizeof(datetime_str), "%Y-%m-%d %H:%M:%S", &timeinfo);

        app.m_ui->updateDateTime(datetime_str);
    }
}

void VPIApplication::onGlobalTouch(lv_event_t* e) {
    // Touch events handled by LVGL (debug logs can be added here if needed)
}
