#include "FileManagerApplication.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_remote.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <algorithm>
#include <unistd.h>

static const char *TAG = "FileManager";

// Default upload directory
#define UPLOAD_DIR "/sdcard/WORKSHOP"

// HTML UI (embedded in code)
static const char *index_html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>LabBuddy File Manager</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container { 
            max-width: 800px; 
            margin: 0 auto; 
            background: white; 
            border-radius: 15px; 
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            overflow: hidden;
        }
        .header { 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white; 
            padding: 30px; 
            text-align: center;
        }
        .header h1 { font-size: 28px; margin-bottom: 5px; }
        .header p { font-size: 14px; opacity: 0.9; }
        .section { padding: 30px; border-bottom: 1px solid #eee; }
        .section:last-child { border-bottom: none; }
        .section h2 { 
            font-size: 18px; 
            margin-bottom: 15px; 
            color: #333;
            display: flex;
            align-items: center;
        }
        .section h2::before { 
            content: '📁'; 
            margin-right: 10px; 
            font-size: 24px;
        }
        .upload-area {
            border: 3px dashed #667eea;
            border-radius: 10px;
            padding: 40px;
            text-align: center;
            background: #f8f9ff;
            cursor: pointer;
            transition: all 0.3s;
        }
        .upload-area:hover { background: #eef1ff; border-color: #764ba2; }
        .upload-area.drag-over { background: #e3e7ff; border-color: #667eea; }
        .upload-icon { font-size: 48px; margin-bottom: 10px; }
        .upload-text { color: #666; margin-bottom: 15px; }
        input[type="file"] { display: none; }
        .btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 12px 30px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 16px;
            transition: transform 0.2s;
        }
        .btn:hover { transform: scale(1.05); }
        .btn:active { transform: scale(0.95); }
        .file-list { margin-top: 15px; }
        .file-item {
            background: #f8f9fa;
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 8px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            transition: all 0.2s;
        }
        .file-item:hover { background: #e9ecef; transform: translateX(5px); }
        .file-info { display: flex; align-items: center; gap: 12px; }
        .file-icon { 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            width: 40px;
            height: 40px;
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-weight: bold;
        }
        .file-name { font-weight: 500; color: #333; }
        .file-size { font-size: 12px; color: #999; margin-top: 2px; }
        .file-actions { display: flex; gap: 8px; }
        .btn-small {
            padding: 8px 16px;
            font-size: 14px;
            border-radius: 20px;
            border: none;
            cursor: pointer;
            transition: all 0.2s;
        }
        .btn-view { background: #4CAF50; color: white; }
        .btn-view:hover { background: #45a049; }
        .btn-delete { background: #f44336; color: white; }
        .btn-delete:hover { background: #da190b; }
        .status { 
            margin-top: 15px; 
            padding: 15px; 
            border-radius: 8px; 
            background: #e3f2fd;
            color: #1976d2;
            display: none;
        }
        .status.success { background: #c8e6c9; color: #388e3c; display: block; }
        .status.error { background: #ffcdd2; color: #c62828; display: block; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📁 LabBuddy File Manager</h1>
            <p>Upload and manage JSON circuit files</p>
        </div>
        
        <div class="section">
            <h2>📤 Upload File</h2>            <div style="margin-bottom:12px;">
                <label style="font-size:14px;color:#555;">📂 Upload to folder: </label>
                <select id="uploadDir" style="padding:6px 12px;border-radius:6px;border:1px solid #ccc;font-size:14px;">
                    <option value="WORKSHOP">WORKSHOP (circuits)</option>
                    <option value="lessons">lessons (บทเรียน)</option>
                </select>
            </div>            <div class="upload-area" id="uploadArea">
                <div class="upload-icon">📄</div>
                <div class="upload-text">Drag & drop JSON file here or click to browse</div>
                <button class="btn" onclick="document.getElementById('fileInput').click()">Upload</button>
                <input type="file" id="fileInput" accept=".json" onchange="uploadFile()">
            </div>
            <div class="status" id="uploadStatus"></div>
        </div>

        <div class="section">
            <h2>💾 SD Card Files</h2>
            <div style="margin-bottom:12px;">
                <label style="font-size:14px;color:#555;">📂 Browse folder: </label>
                <select id="browseDir" style="padding:6px 12px;border-radius:6px;border:1px solid #ccc;font-size:14px;" onchange="loadFiles()">
                    <option value="lessons">lessons (บทเรียน)</option>
                    <option value="WORKSHOP">WORKSHOP (circuits)</option>
                </select>
            </div>
            <div class="file-list" id="fileList">Loading...</div>
            <div class="status" style="text-align: center; margin-top: 20px;">
                <strong>Status:</strong> <span id="fileCount">0</span> file(s) on SD card
            </div>
        </div>
    </div>

    <script>
        const uploadArea = document.getElementById('uploadArea');
        const fileInput = document.getElementById('fileInput');
        const uploadStatus = document.getElementById('uploadStatus');
        const fileList = document.getElementById('fileList');
        const fileCount = document.getElementById('fileCount');

        // Drag and drop
        uploadArea.ondragover = (e) => { e.preventDefault(); uploadArea.classList.add('drag-over'); };
        uploadArea.ondragleave = () => uploadArea.classList.remove('drag-over');
        uploadArea.ondrop = (e) => {
            e.preventDefault();
            uploadArea.classList.remove('drag-over');
            if (e.dataTransfer.files.length > 0) {
                fileInput.files = e.dataTransfer.files;
                uploadFile();
            }
        };
        uploadArea.onclick = () => fileInput.click();

        async function uploadFile() {
            const file = fileInput.files[0];
            if (!file) return;

            const dir = document.getElementById('uploadDir').value;
            const formData = new FormData();
            formData.append('file', file);

            try {
                const response = await fetch('/upload?dir=' + encodeURIComponent(dir), { method: 'POST', body: formData });
                const result = await response.json();
                
                if (result.success) {
                    uploadStatus.className = 'status success';
                    uploadStatus.textContent = `✅ File uploaded: ${file.name}`;
                    loadFiles();
                } else {
                    uploadStatus.className = 'status error';
                    uploadStatus.textContent = `❌ Upload failed: ${result.message || 'Unknown error'}`;
                }
            } catch (error) {
                uploadStatus.className = 'status error';
                uploadStatus.textContent = '❌ Upload failed: ' + error.message;
            }

            fileInput.value = '';
        }

        async function loadFiles() {
            const dir = document.getElementById('browseDir').value;
            try {
                const response = await fetch('/files?dir=' + encodeURIComponent(dir));
                const data = await response.json();
                
                if (data.files && data.files.length > 0) {
                    fileList.innerHTML = data.files.map(file => `
                        <div class="file-item">
                            <div class="file-info">
                                <div class="file-icon">JSON</div>
                                <div>
                                    <div class="file-name">${file.name}</div>
                                    <div class="file-size">${(file.size / 1024).toFixed(1)} KB</div>
                                </div>
                            </div>
                            <div class="file-actions">
                                ${dir === 'lessons'
                                    ? `<button class="btn-small btn-view" onclick="launchLesson('${file.name}')">🚀 Launch</button>`
                                    : `<button class="btn-small btn-view" onclick="viewFile('${file.name}')">📺 Render</button>`
                                }
                                <button class="btn-small btn-delete" onclick="deleteFile('${file.name}')">🗑️</button>
                            </div>
                        </div>
                    `).join('');
                    fileCount.textContent = data.files.length;
                } else {
                    fileList.innerHTML = '<div style="text-align: center; color: #999; padding: 20px;">No files found</div>';
                    fileCount.textContent = '0';
                }
            } catch (error) {
                fileList.innerHTML = '<div style="text-align: center; color: #f44336; padding: 20px;">Error loading files</div>';
            }
        }

        async function launchLesson(filename) {
            const dir = document.getElementById('browseDir').value;
            try {
                const response = await fetch(`/launch?file=${encodeURIComponent(filename)}&dir=${encodeURIComponent(dir)}`);
                const result = await response.json();
                if (result.success) {
                    showMessage('🚀 ' + result.message, 'success');
                } else {
                    showMessage('❌ ' + (result.message || 'Launch failed'), 'error');
                }
            } catch (error) {
                showMessage('❌ Launch failed: ' + error.message, 'error');
            }
        }

        async function viewFile(filename) {
            const dir = document.getElementById('browseDir').value;
            try {
                const response = await fetch(`/view?file=${encodeURIComponent(filename)}&dir=${encodeURIComponent(dir)}`);
                const result = await response.json();
                
                if (result.success) {
                    showMessage(result.message, 'success');
                } else {
                    showMessage(result.message || 'Render failed', 'error');
                }
            } catch (error) {
                showMessage('Failed to render file', 'error');
            }
        }

        function downloadFile(filename) {
            window.location.href = '/file/' + filename;
        }

        async function deleteFile(filename) {
            if (!confirm(`Delete ${filename}?`)) return;

            try {
                const response = await fetch(`/delete/${filename}`, { method: 'DELETE' });
                const result = await response.json();
                
                if (result.success) {
                    loadFiles();
                } else {
                    alert('Delete failed: ' + result.message);
                }
            } catch (error) {
                alert('Delete failed: ' + error.message);
            }
        }

        // Load files on page load
        loadFiles();
    </script>
</body>
</html>
)HTML";

// Singleton instance
FileManagerApplication &FileManagerApplication::getInstance()
{
    static FileManagerApplication instance;
    return instance;
}

// Constructor
FileManagerApplication::FileManagerApplication()
    : m_sysMgr(nullptr), m_server(nullptr), m_viewer(nullptr), m_initialized(false), m_wifi_connected(false), m_retry_count(0)
{
    memset(m_ip_address, 0, sizeof(m_ip_address));
}

// Destructor
FileManagerApplication::~FileManagerApplication()
{
    stop();
    if (m_viewer)
    {
        delete m_viewer;
        m_viewer = nullptr;
    }
}

// Get default WiFi config
FileManagerApplication::WiFiConfig FileManagerApplication::getDefaultWiFiConfig()
{
    return WiFiConfig{
        .ssid = "AESFIBER",
        .password = "29052552",
        .max_retry = 5,
        .connect_timeout_ms = 10000};
}

// Initialize
esp_err_t FileManagerApplication::init(SystemManager &sysMgr, const WiFiConfig *wifi_config)
{
    if (m_initialized)
    {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }

    m_sysMgr = &sysMgr;

    // Copy WiFi config
    if (wifi_config)
    {
        m_wifi_config = *wifi_config;
    }
    else
    {
        m_wifi_config = getDefaultWiFiConfig();
    }

    // Ensure SD card is mounted
    if (!m_sysMgr->isSDCardMounted())
    {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_FAIL;
    }

    // Create upload directory
    struct stat st;
    if (stat(UPLOAD_DIR, &st) != 0)
    {
        ESP_LOGI(TAG, "Creating directory: %s", UPLOAD_DIR);
        if (mkdir(UPLOAD_DIR, 0755) != 0)
        {
            ESP_LOGE(TAG, "Failed to create directory: %s (errno: %d)", UPLOAD_DIR, errno);
            return ESP_FAIL;
        }
    }

    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create FileViewerUI (UI will be created lazily on first render)
    ESP_LOGI(TAG, "Creating FileViewerUI...");
    m_viewer = new FileViewerUI();
    if (!m_viewer)
    {
        ESP_LOGE(TAG, "Failed to allocate FileViewerUI");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "FileViewerUI allocated (UI creation deferred to LVGL task)");

    m_initialized = true;
    ESP_LOGI(TAG, "FileManager initialized");

    return ESP_OK;
}

// WiFi event handler (static wrapper)
void FileManagerApplication::wifi_event_handler(void *arg, esp_event_base_t event_base,
                                                int32_t event_id, void *event_data)
{
    FileManagerApplication *app = static_cast<FileManagerApplication *>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WiFi station started, connecting...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "WiFi disconnected (reason: %d)", disconn->reason);

        if (app->m_retry_count < app->m_wifi_config.max_retry)
        {
            esp_wifi_connect();
            app->m_retry_count++;
            ESP_LOGI(TAG, "Retry connecting (%d/%d)", app->m_retry_count, app->m_wifi_config.max_retry);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to connect after %d attempts", app->m_wifi_config.max_retry);
            app->m_wifi_connected = false;
        }
    }
}

// IP event handler (static wrapper)
void FileManagerApplication::ip_event_handler(void *arg, esp_event_base_t event_base,
                                              int32_t event_id, void *event_data)
{
    FileManagerApplication *app = static_cast<FileManagerApplication *>(arg);

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(app->m_ip_address, sizeof(app->m_ip_address), IPSTR, IP2STR(&event->ip_info.ip));

        ESP_LOGI(TAG, "Got IP: %s", app->m_ip_address);
        ESP_LOGI(TAG, "===================================");
        ESP_LOGI(TAG, "Web UI: http://%s", app->m_ip_address);
        ESP_LOGI(TAG, "===================================");

        app->m_wifi_connected = true;
        app->m_retry_count = 0;

        // Update IP label on display
        if (app->m_viewer)
        {
            lv_lock();
            app->m_viewer->updateIP(app->m_ip_address);
            lv_unlock();
        }
    }
}

// Initialize WiFi
esp_err_t FileManagerApplication::initWiFi()
{
    ESP_LOGI(TAG, "Initializing WiFi (ESP-HOSTED)...");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers with 'this' as argument
    esp_event_handler_instance_t instance_wifi, instance_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &FileManagerApplication::wifi_event_handler,
        this, &instance_wifi));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &FileManagerApplication::ip_event_handler,
        this, &instance_ip));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, m_wifi_config.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, m_wifi_config.password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi: %s...", m_wifi_config.ssid);

    // Manual connect attempt
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "esp_wifi_connect() returned: %s", esp_err_to_name(ret));
    }

    return ESP_OK;
}

// HTTP Handler: Root (HTML UI)
esp_err_t FileManagerApplication::root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// HTTP Handler: List files
esp_err_t FileManagerApplication::list_handler(httpd_req_t *req)
{
    // Allow caller to select directory via ?dir=lessons or ?dir=WORKSHOP
    char list_dir[64];
    strlcpy(list_dir, UPLOAD_DIR, sizeof(list_dir)); // default = /sdcard/WORKSHOP
    {
        char query[128] = {0};
        if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
        {
            char dir_param[64] = {0};
            if (httpd_query_key_value(query, "dir", dir_param, sizeof(dir_param)) == ESP_OK && strlen(dir_param) > 0)
            {
                bool ok = true;
                for (int i = 0; dir_param[i]; i++)
                {
                    char c = dir_param[i];
                    if (!isalnum((unsigned char)c) && c != '_' && c != '-')
                    {
                        ok = false;
                        break;
                    }
                }
                if (ok)
                    snprintf(list_dir, sizeof(list_dir), "/sdcard/%s", dir_param);
            }
        }
    }

    DIR *dir = opendir(list_dir);
    if (!dir)
    {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "{\"error\":\"Failed to open directory\"}");
        return ESP_FAIL;
    }

    char *json = (char *)malloc(4096);
    if (!json)
    {
        closedir(dir);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }

    strcpy(json, "{\"files\":[");

    struct dirent *entry;
    struct stat st;
    bool first = true;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", list_dir, entry->d_name);

            if (stat(filepath, &st) == 0)
            {
                if (!first)
                    strcat(json, ",");

                char item[512];
                snprintf(item, sizeof(item),
                         "{\"name\":\"%s\",\"size\":%ld}",
                         entry->d_name, st.st_size);
                strcat(json, item);
                first = false;
            }
        }
    }

    closedir(dir);
    strcat(json, "]}");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json);
    free(json);

    return ESP_OK;
}

// HTTP Handler: Upload file
esp_err_t FileManagerApplication::upload_handler(httpd_req_t *req)
{
    char filepath[256];
    char filename[128] = {0};
    FILE *file = NULL;

    // Parse optional ?dir= query parameter to choose upload subdirectory
    char upload_dir[64];
    strlcpy(upload_dir, UPLOAD_DIR, sizeof(upload_dir)); // default
    {
        char query[128] = {0};
        if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
        {
            char dir_param[64] = {0};
            if (httpd_query_key_value(query, "dir", dir_param, sizeof(dir_param)) == ESP_OK && strlen(dir_param) > 0)
            {
                // Sanitize: allow alphanumeric, underscore, hyphen only
                bool ok = true;
                for (int i = 0; dir_param[i]; i++)
                {
                    char c = dir_param[i];
                    if (!isalnum((unsigned char)c) && c != '_' && c != '-')
                    {
                        ok = false;
                        break;
                    }
                }
                if (ok)
                    snprintf(upload_dir, sizeof(upload_dir), "/sdcard/%s", dir_param);
            }
        }
    }

    ESP_LOGI(TAG, "Upload request: content_len=%d dir=%s", req->content_len, upload_dir);

    char buf[1024];
    int received;
    int remaining = req->content_len;

    // Read first chunk
    received = httpd_req_recv(req, buf, std::min(remaining, (int)sizeof(buf)));
    if (received <= 0)
    {
        ESP_LOGE(TAG, "Failed to receive data");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive data");
        return ESP_FAIL;
    }

    // Parse filename from multipart data
    char *filename_start = strstr(buf, "filename=\"");
    if (filename_start)
    {
        filename_start += 10;
        char *filename_end = strchr(filename_start, '"');
        if (filename_end)
        {
            int len = filename_end - filename_start;
            if (len > 0 && len < (int)sizeof(filename))
            {
                strncpy(filename, filename_start, len);
                filename[len] = '\0';

                // Convert to uppercase
                for (int i = 0; filename[i]; i++)
                {
                    filename[i] = toupper(filename[i]);
                }
            }
        }
    }

    if (strlen(filename) == 0)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No filename found");
        return ESP_FAIL;
    }

    snprintf(filepath, sizeof(filepath), "%s/%s", upload_dir, filename);
    ESP_LOGI(TAG, "Uploading: %s", filepath);

    // Find file data start
    char *data_start = strstr(buf, "\r\n\r\n");
    if (!data_start)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid multipart data");
        return ESP_FAIL;
    }
    data_start += 4;

    // Open file
    file = fopen(filepath, "w");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to create file");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    // Write first chunk
    int first_chunk_size = received - (data_start - buf);
    fwrite(data_start, 1, first_chunk_size, file);
    remaining -= received;

    // Receive remaining chunks
    while (remaining > 0)
    {
        received = httpd_req_recv(req, buf, std::min(remaining, (int)sizeof(buf)));

        // Allow early connection close (multipart boundary tolerance)
        if (received == 0 && remaining < 100)
        {
            ESP_LOGW(TAG, "Connection closed with %d bytes remaining (multipart boundary)", remaining);
            break;
        }

        if (received <= 0)
        {
            ESP_LOGE(TAG, "Failed to receive chunk: received=%d, remaining=%d", received, remaining);
            fclose(file);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive data");
            return ESP_FAIL;
        }

        // Remove boundary marker in last chunk
        if (remaining == received)
        {
            for (int i = received - 1; i > 0; i--)
            {
                if (buf[i] == '-' && buf[i - 1] == '-')
                {
                    received = i - 1;
                    break;
                }
            }
        }

        fwrite(buf, 1, received, file);
        remaining -= received;
    }

    fclose(file);
    ESP_LOGI(TAG, "File uploaded: %s", filepath);

    // Send response
    char response[256];
    snprintf(response, sizeof(response),
             "{\"success\":true,\"message\":\"Uploaded: %s\"}", filename);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response);

    return ESP_OK;
}

// HTTP Handler: Download file
esp_err_t FileManagerApplication::download_handler(httpd_req_t *req)
{
    const char *filename = req->uri + 6; // Skip "/file/"

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/octet-stream");

    char buf[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buf, 1, sizeof(buf), file)) > 0)
    {
        if (httpd_resp_send_chunk(req, buf, read_bytes) != ESP_OK)
        {
            fclose(file);
            return ESP_FAIL;
        }
    }

    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0); // End response

    ESP_LOGI(TAG, "File downloaded: %s", filename);
    return ESP_OK;
}

// HTTP Handler: View/Render file on LCD
esp_err_t FileManagerApplication::view_handler(httpd_req_t *req)
{
    // Parse query string: /view?file=FILENAME.JSON&dir=lessons
    char query[256];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing file parameter");
        return ESP_FAIL;
    }

    char filename[128];
    if (httpd_query_key_value(query, "file", filename, sizeof(filename)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid file parameter");
        return ESP_FAIL;
    }

    // Optional dir param (default = WORKSHOP)
    char view_dir[64];
    strlcpy(view_dir, UPLOAD_DIR, sizeof(view_dir));
    {
        char dir_param[64] = {0};
        if (httpd_query_key_value(query, "dir", dir_param, sizeof(dir_param)) == ESP_OK && strlen(dir_param) > 0)
        {
            bool ok = true;
            for (int i = 0; dir_param[i]; i++)
            {
                char c = dir_param[i];
                if (!isalnum((unsigned char)c) && c != '_' && c != '-')
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
                snprintf(view_dir, sizeof(view_dir), "/sdcard/%s", dir_param);
        }
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", view_dir, filename);

    ESP_LOGI(TAG, "View request: %s", filepath);

    // Get FileManagerApplication instance
    FileManagerApplication &app = FileManagerApplication::getInstance();

    if (!app.m_viewer)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Viewer not initialized");
        return ESP_FAIL;
    }

    // Render file on LCD using full path
    bool success = app.m_viewer->renderFilePath(filepath);

    // Get debug info from renderer
    std::string debugInfo = app.m_viewer->getLastDebugInfo();

    // Send JSON response (include debug info)
    httpd_resp_set_type(req, "application/json");
    if (success)
    {
        // Build response with debug info
        std::string resp = "{\"success\":true,\"message\":\"Rendered: ";
        resp += filename;
        resp += "\",\"debug\":";
        resp += debugInfo.empty() ? "null" : debugInfo;
        resp += "}";
        httpd_resp_sendstr(req, resp.c_str());
        ESP_LOGI(TAG, "File rendered: %s", filename);
        return ESP_OK;
    }
    else
    {
        std::string resp = "{\"success\":false,\"message\":\"Render failed: ";
        resp += filename;
        resp += "\",\"debug\":";
        resp += debugInfo.empty() ? "null" : debugInfo;
        resp += "}";
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, resp.c_str());
        ESP_LOGE(TAG, "Render failed: %s", filename);
        return ESP_FAIL;
    }
}

// HTTP Handler: Delete file
esp_err_t FileManagerApplication::delete_handler(httpd_req_t *req)
{
    const char *filename = req->uri + 8; // Skip "/delete/"

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);

    if (unlink(filepath) == 0)
    {
        ESP_LOGI(TAG, "File deleted: %s", filepath);

        char response[256];
        snprintf(response, sizeof(response),
                 "{\"success\":true,\"message\":\"Deleted: %s\"}", filename);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response);
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to delete: %s", filepath);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "{\"success\":false,\"message\":\"Delete failed - file not found or permission error\"}");
        return ESP_FAIL;
    }
}

// HTTP Handler: Launch lesson on device via LessonPlayer
// GET /launch?file=L001_NOT_GATE.JSON&dir=lessons
esp_err_t FileManagerApplication::launch_handler(httpd_req_t *req)
{
    char query[256];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing file parameter");
        return ESP_FAIL;
    }

    char filename[128];
    if (httpd_query_key_value(query, "file", filename, sizeof(filename)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid file parameter");
        return ESP_FAIL;
    }

    // dir param (default = lessons)
    char dir_param[64] = "lessons";
    httpd_query_key_value(query, "dir", dir_param, sizeof(dir_param));

    // Sanitize dir
    for (int i = 0; dir_param[i]; i++)
    {
        char c = dir_param[i];
        if (!isalnum((unsigned char)c) && c != '_' && c != '-')
        {
            strlcpy(dir_param, "lessons", sizeof(dir_param));
            break;
        }
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/sdcard/%s/%s", dir_param, filename);
    ESP_LOGI(TAG, "Launch lesson: %s", filepath);

    // Load lesson (file I/O — must be outside LVGL lock)
    LessonPlayer &lp = LessonPlayer::getInstance();
    bool loaded = lp.loadLesson(filepath);

    httpd_resp_set_type(req, "application/json");
    if (loaded)
    {
        // show() acquires LVGL lock internally
        lp.show();
        const char *resp = "{\"success\":true,\"message\":\"Lesson launched on device\"}";
        httpd_resp_sendstr(req, resp);
        ESP_LOGI(TAG, "Lesson launched: %s", filepath);
    }
    else
    {
        char resp[256];
        snprintf(resp, sizeof(resp),
                 "{\"success\":false,\"message\":\"Failed to load: %s\"}", filepath);
        httpd_resp_sendstr(req, resp);
        ESP_LOGW(TAG, "Lesson load failed: %s", filepath);
    }

    return ESP_OK;
}

// Register HTTP handlers
void FileManagerApplication::registerHTTPHandlers()
{
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &root_uri);

    httpd_uri_t files_uri = {
        .uri = "/files",
        .method = HTTP_GET,
        .handler = list_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &files_uri);

    httpd_uri_t upload_uri = {
        .uri = "/upload",
        .method = HTTP_POST,
        .handler = upload_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &upload_uri);

    httpd_uri_t download_uri = {
        .uri = "/file/*",
        .method = HTTP_GET,
        .handler = download_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &download_uri);

    httpd_uri_t delete_uri = {
        .uri = "/delete/*",
        .method = HTTP_DELETE,
        .handler = delete_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &delete_uri);

    httpd_uri_t view_uri = {
        .uri = "/view",
        .method = HTTP_GET,
        .handler = view_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &view_uri);

    httpd_uri_t launch_uri = {
        .uri = "/launch",
        .method = HTTP_GET,
        .handler = launch_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(m_server, &launch_uri);
}

// Start HTTP server
esp_err_t FileManagerApplication::startHTTPServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 9;
    config.stack_size = 16384;                      // Increased from default 4096 to handle large debug responses
    config.uri_match_fn = httpd_uri_match_wildcard; // Required for /delete/* and /file/* patterns

    ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);

    if (httpd_start(&m_server, &config) == ESP_OK)
    {
        registerHTTPHandlers();
        ESP_LOGI(TAG, "HTTP server started successfully");
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");
    return ESP_FAIL;
}

// Stop HTTP server
void FileManagerApplication::stopHTTPServer()
{
    if (m_server)
    {
        httpd_stop(m_server);
        m_server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }
}

// Start application
esp_err_t FileManagerApplication::start(bool show_ui)
{
    if (!m_initialized)
    {
        ESP_LOGE(TAG, "Not initialized");
        return ESP_FAIL;
    }

    // Initialize WiFi
    esp_err_t ret = initWiFi();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi initialization failed");
        return ret;
    }

    // Wait for WiFi connection
    int wait_time = 0;
    while (!m_wifi_connected && wait_time < m_wifi_config.connect_timeout_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        wait_time += 100;
    }

    if (!m_wifi_connected)
    {
        ESP_LOGW(TAG, "WiFi not connected yet, but starting HTTP server anyway");
    }

    // Start HTTP server
    ret = startHTTPServer();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP server start failed");
        return ret;
    }

    ESP_LOGI(TAG, "FileManager application started");

    // Show initial UI on display (skip when HMINavigator owns the display)
    if (show_ui && m_viewer)
    {
        ESP_LOGI(TAG, "Creating display UI...");
        lv_lock();
        m_viewer->create(lv_screen_active());
        lv_unlock();
        ESP_LOGI(TAG, "Display UI created");
    }
    else
    {
        ESP_LOGI(TAG, "FileManager UI suppressed (headless mode)");
    }

    return ESP_OK;
}

// Stop application
void FileManagerApplication::stop()
{
    stopHTTPServer();

    if (m_wifi_connected)
    {
        esp_wifi_stop();
        m_wifi_connected = false;
    }

    ESP_LOGI(TAG, "FileManager application stopped");
}
