#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "SystemManager.h"
#include "FileViewerUI.h"

/**
 * @brief FileManagerApplication - Web-based file manager
 *
 * This application provides:
 * - WiFi connectivity (via ESP-HOSTED on ESP32-C6)
 * - HTTP server with web UI
 * - File upload/download/delete via web interface
 * - JSON circuit file management
 */
class FileManagerApplication
{
public:
    /**
     * @brief WiFi configuration
     */
    struct WiFiConfig
    {
        const char *ssid;
        const char *password;
        int max_retry;
        int connect_timeout_ms;
    };

    /**
     * @brief Get FileManagerApplication singleton instance
     */
    static FileManagerApplication &getInstance();

    /**
     * @brief Initialize the file manager application
     * @param sysMgr Reference to SystemManager
     * @param wifi_config WiFi configuration (nullptr = use defaults)
     * @return ESP_OK on success
     */
    esp_err_t init(SystemManager &sysMgr, const WiFiConfig *wifi_config = nullptr);

    /**
     * @brief Start WiFi and HTTP server
     * @return ESP_OK on success
     */
    esp_err_t start();

    /**
     * @brief Stop HTTP server and disconnect WiFi
     */
    void stop();

    /**
     * @brief Check if WiFi is connected
     */
    bool isWiFiConnected() const { return m_wifi_connected; }

    /**
     * @brief Get current IP address
     */
    const char *getIPAddress() const { return m_ip_address; }

    /**
     * @brief Get HTTP server handle
     */
    httpd_handle_t getServerHandle() const { return m_server; }

    /**
     * @brief Get default WiFi config
     */
    static WiFiConfig getDefaultWiFiConfig();

private:
    FileManagerApplication();
    ~FileManagerApplication();
    FileManagerApplication(const FileManagerApplication &) = delete;
    FileManagerApplication &operator=(const FileManagerApplication &) = delete;

    // Private initialization methods
    esp_err_t initWiFi();
    esp_err_t startHTTPServer();
    void stopHTTPServer();

    // HTTP handler registration
    void registerHTTPHandlers();

    // Static callback wrappers for C API
    static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);
    static void ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data);

    // HTTP handlers (implemented as static C functions)
    static esp_err_t root_handler(httpd_req_t *req);
    static esp_err_t upload_handler(httpd_req_t *req);
    static esp_err_t list_handler(httpd_req_t *req);
    static esp_err_t download_handler(httpd_req_t *req);
    static esp_err_t delete_handler(httpd_req_t *req);
    static esp_err_t view_handler(httpd_req_t *req);

    SystemManager *m_sysMgr;
    WiFiConfig m_wifi_config;
    httpd_handle_t m_server;
    FileViewerUI *m_viewer;

    bool m_initialized;
    bool m_wifi_connected;
    char m_ip_address[16];

    int m_retry_count;
};
