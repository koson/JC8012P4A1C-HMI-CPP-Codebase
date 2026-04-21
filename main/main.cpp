#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// C++ VPI HMI UI
#include "VPIHMIUI.h"

// VPI Document & Logging (C++)
#include "VPIDocument.h"
#include "DataLogger.h"
#include "VPIDocumentCWrapper.h"

// VPI HMI Components (C)
extern "C" {
    #include "uart_receiver.h"
    #include "vpi_data_manager.h"
    #include "sdmmc_cmd.h"  // SD card info functions
    #include "bsp/touch.h"   // BSP touchscreen
    // #include "sd_card_test.h"  // Replaced by DataLogger
    
    // BSP SD card handle (from bsp/esp-bsp.h)
    extern sdmmc_card_t *bsp_sdcard;
}

// Global UI instance (accessed by data callbacks)
static VPIHMIUI* g_vpi_hmi_ui = nullptr;

// Global VPI Document & Logger instances
static VPIDocument* g_vpi_document = nullptr;
static DataLogger* g_data_logger = nullptr;

// ========== UI Event Handlers ==========

/**
 * @brief Global touch event handler for debugging
 */
static void global_touch_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    // Touch events handled by LVGL (debug logs removed)
}

/**
 * @brief LOG button event handler - Toggle logging on/off
 */
static void log_button_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));  // LVGL v9 API + C++ cast
    
    // Debug: Log ALL events received by button
    ESP_LOGI("main", "LOG button event received: code=%d", code);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI("main", ">>> LOG button CLICKED event detected!");
        bool is_checked = lv_obj_has_state(btn, LV_STATE_CHECKED);
        
        ESP_LOGI("main", "Button state: %s", is_checked ? "CHECKED" : "UNCHECKED");
        
        if (g_data_logger) {
            if (is_checked) {
                // Button now checked = START logging
                ESP_LOGI("main", "LOG button clicked - STARTING logging");
                ESP_LOGI("main", "Calling g_data_logger->startLogging()...");
                
                bool started = g_data_logger->startLogging();
                
                ESP_LOGI("main", "startLogging() returned: %s", started ? "true" : "false");
                
                if (started) {
                    g_vpi_hmi_ui->setLoggingState(true);
                    ESP_LOGI("main", "✓ CSV logging started");
                } else {
                    ESP_LOGE("main", "✗ Failed to start logging (already running?)");
                    // Revert button state
                    lv_obj_remove_state(btn, LV_STATE_CHECKED);
                    g_vpi_hmi_ui->setLoggingState(false);
                    ESP_LOGI("main", "Button state reverted to UNCHECKED");
                }
            } else {
                // Button unchecked = STOP logging
                ESP_LOGI("main", "LOG button clicked - STOPPING logging");
                g_data_logger->stopLogging();
                g_vpi_hmi_ui->setLoggingState(false);
                ESP_LOGI("main", "✓ CSV logging stopped");
            }
        } else {
            ESP_LOGW("main", "DataLogger not initialized!");
            // Revert button state
            lv_obj_remove_state(btn, LV_STATE_CHECKED);
        }
    }
}

/**
 * @brief DateTime update timer callback - Update every 1 second
 */
static void datetime_timer_cb(lv_timer_t* timer)
{
    if (g_vpi_hmi_ui) {
        // Get current time
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Format: "YYYY-MM-DD HH:MM:SS"
        char datetime_str[32];
        strftime(datetime_str, sizeof(datetime_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
        
        g_vpi_hmi_ui->updateDateTime(datetime_str);
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI("main", "Starting app_main()");
    
    // Initialize display with BSP defaults (no rotation yet)
    ESP_LOGI("main", "Initializing display with BSP...");
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * 100,  // Partial buffer (100 lines = 160KB)
        .double_buffer = 0,  // Single buffer for internal RAM
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,  // Use SPIRAM (external RAM)
            .sw_rotate = true,
        }
    };
    
    ESP_LOGI("main", "Calling bsp_display_start_with_config()");
    bsp_display_start_with_config(&cfg);
    
    ESP_LOGI("main", "Display started, turning on backlight");
    bsp_display_backlight_on();

    ESP_LOGI("main", "Display initialized");

    // ========== Check Touchscreen Status (BSP already initialized it) ==========
    // Note: bsp_display_start_with_config() calls bsp_touch_new() internally
    // We just check if it succeeded by getting the input device
    lv_indev_t* touch_indev = bsp_display_get_input_dev();
    if (touch_indev == NULL) {
        ESP_LOGW("main", "⚠️ Touchscreen not available - LOG button will not work");
        ESP_LOGW("main", "⚠️ System will continue in display-only mode");
    } else {
        ESP_LOGI("main", "✓ Touchscreen available (indev=%p)", touch_indev);
    }

    // Rotate display to landscape (90° clockwise = portrait → landscape)
    lv_display_t* disp = lv_display_get_default();
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);  // Rotate 270° for landscape mode
    ESP_LOGI("main", "Display rotated to landscape (270°)");

    // Check touchscreen/input devices
    ESP_LOGI("main", "===========================================");
    ESP_LOGI("main", "  Checking Input Devices (Touchscreen)");
    ESP_LOGI("main", "===========================================");
    
    lv_indev_t* indev = lv_indev_get_next(NULL);
    int indev_count = 0;
    while (indev) {
        indev_count++;
        lv_indev_type_t type = lv_indev_get_type(indev);
        const char* type_str = (type == LV_INDEV_TYPE_POINTER) ? "POINTER/TOUCH" :
                               (type == LV_INDEV_TYPE_KEYPAD) ? "KEYPAD" :
                               (type == LV_INDEV_TYPE_BUTTON) ? "BUTTON" :
                               (type == LV_INDEV_TYPE_ENCODER) ? "ENCODER" : "UNKNOWN";
        ESP_LOGI("main", "  Input Device #%d: Type=%s (%d)", indev_count, type_str, type);
        indev = lv_indev_get_next(indev);
    }
    
    if (indev_count == 0) {
        ESP_LOGE("main", "✗ NO INPUT DEVICES FOUND!");
        ESP_LOGE("main", "✗ Touchscreen is NOT initialized!");
        ESP_LOGE("main", "  Check BSP touchscreen initialization...");
    } else {
        ESP_LOGI("main", "✓ Found %d input device(s)", indev_count);
    }
    ESP_LOGI("main", "===========================================");


    // Lock display with timeout
    if (bsp_display_lock(portMAX_DELAY)) {
        ESP_LOGI("main", "Display locked, creating VPI HMI UI...");
    } else {
        ESP_LOGE("main", "Failed to lock display");
        return;
    }

    // ========== VPI HMI System Initialization ==========
    ESP_LOGI("main", "===========================================");
    ESP_LOGI("main", "  VPI HMI System for ESP32-P4");
    ESP_LOGI("main", "  Vacuum Pressure Impregnation Monitor");
    ESP_LOGI("main", "  C++ OOP + LVGL v9.2.2");
    ESP_LOGI("main", "===========================================");

    // Initialize UART receiver
    ESP_LOGI("main", "Initializing UART receiver...");
    ESP_ERROR_CHECK(uart_receiver_init());
    
    // Initialize VPI data manager
    ESP_LOGI("main", "Initializing VPI data manager...");
    ESP_ERROR_CHECK(vpi_data_manager_init());

    // Create VPI HMI UI (C++ OOP, landscape layout)
    // Create VPI HMI UI (C++ OOP, landscape layout)
    ESP_LOGI("main", "Creating VPI HMI UI (C++ OOP, landscape 1280x800)...");
    g_vpi_hmi_ui = new VPIHMIUI();

    // Add global touch event handler to screen for debugging
    lv_obj_t* screen = lv_scr_act();
    lv_obj_add_event_cb(screen, global_touch_event_cb, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(screen, global_touch_event_cb, LV_EVENT_RELEASED, nullptr);
    ESP_LOGI("main", "✓ Global touch event handler registered");

    // Setup LOG button event handler (already inside display lock from line 143)
    lv_obj_t* log_btn = g_vpi_hmi_ui->getLogButton();
    if (log_btn) {
        // Get header panel for debugging
        lv_obj_t* header = lv_obj_get_parent(log_btn);
        if (header) {
            lv_coord_t hx = lv_obj_get_x(header);
            lv_coord_t hy = lv_obj_get_y(header);
            lv_coord_t hw = lv_obj_get_width(header);
            lv_coord_t hh = lv_obj_get_height(header);
            ESP_LOGI("main", "  Header panel: X=%d, Y=%d, W=%d, H=%d", hx, hy, hw, hh);
        }
        
        // Register for ALL events to debug
        lv_obj_add_event_cb(log_btn, log_button_event_cb, LV_EVENT_ALL, nullptr);
        ESP_LOGI("main", "✓ LOG button event handler registered (btn=%p)", log_btn);
        
        // Print button position for debugging (relative to parent)
        lv_coord_t x = lv_obj_get_x(log_btn);
        lv_coord_t y = lv_obj_get_y(log_btn);
        lv_coord_t w = lv_obj_get_width(log_btn);
        lv_coord_t h = lv_obj_get_height(log_btn);
        ESP_LOGI("main", "  Button position (relative): X=%d, Y=%d, W=%d, H=%d", x, y, w, h);
        
        // Get absolute coordinates
        lv_area_t coords;
        lv_obj_get_coords(log_btn, &coords);
        ESP_LOGI("main", "  Button absolute: X1=%d, Y1=%d, X2=%d, Y2=%d", 
                 coords.x1, coords.y1, coords.x2, coords.y2);
        ESP_LOGI("main", "  Touch this area to trigger button!");
    } else {
        ESP_LOGE("main", "✗ LOG button is NULL!");
    }
    
    // Create DateTime update timer (1000ms = 1 second)
    lv_timer_t* datetime_timer = lv_timer_create(datetime_timer_cb, 1000, nullptr);
    if (datetime_timer) {
        ESP_LOGI("main", "✓ DateTime update timer created (1s interval)");
        // Trigger immediately for initial update
        lv_timer_ready(datetime_timer);
    }

    ESP_LOGI("main", "VPI HMI UI created, unlocking display");
    bsp_display_unlock();
    
    // Start UART receiver
    ESP_LOGI("main", "Starting UART receiver...");
    ESP_ERROR_CHECK(uart_receiver_start());
    
    // Start VPI data manager
    ESP_LOGI("main", "Starting VPI data manager...");
    ESP_ERROR_CHECK(vpi_data_manager_start());

    ESP_LOGI("main", "VPI HMI System ready!");
    ESP_LOGI("main", "Waiting for data from Gateway...");
    
    // ========== VPI Data Logger Setup ==========
    vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for system to stabilize
    
    ESP_LOGI("main", "===========================================");
    ESP_LOGI("main", "  Initializing CSV Data Logger");
    ESP_LOGI("main", "===========================================");
    
    // Mount SD Card first
    ESP_LOGI("main", "Mounting SD card...");
    esp_err_t ret = bsp_sdcard_mount();
    if (ret != ESP_OK) {
        ESP_LOGE("main", "Failed to mount SD card (%s)", esp_err_to_name(ret));
        ESP_LOGE("main", "CSV logging will be disabled!");
    } else {
        ESP_LOGI("main", "SD card mounted successfully!");
        
        // Print SD card info
        if (bsp_sdcard != NULL) {
            sdmmc_card_print_info(stdout, bsp_sdcard);
        }
        
        // Wait for filesystem to stabilize
        vTaskDelay(pdMS_TO_TICKS(1000));  // Increased delay
        
        // Verify mount point is accessible
        DIR* dir = opendir("/sdcard");
        if (dir == NULL) {
            ESP_LOGE("main", "Cannot access /sdcard: %s", strerror(errno));
            ret = ESP_FAIL;
        } else {
            closedir(dir);
            ESP_LOGI("main", "SD card ready for use");
            
            // List files in root directory
            ESP_LOGI("main", "===========================================");
            ESP_LOGI("main", "  SD Card Contents");
            ESP_LOGI("main", "===========================================");
            
            dir = opendir("/sdcard");
            if (dir != NULL) {
                ESP_LOGI("main", "📁 /sdcard/:");
                struct dirent* entry;
                int file_count = 0;
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_type == DT_DIR) {
                        ESP_LOGI("main", "  📂 %s/", entry->d_name);
                    } else {
                        ESP_LOGI("main", "  📄 %s", entry->d_name);
                    }
                    file_count++;
                }
                closedir(dir);
                ESP_LOGI("main", "Total: %d items", file_count);
            }
            
            // List files in logs directory if exists
            DIR* logs_dir = opendir("/sdcard/logs");
            if (logs_dir != NULL) {
                ESP_LOGI("main", "📁 /sdcard/logs/:");
                struct dirent* entry;
                int log_count = 0;
                while ((entry = readdir(logs_dir)) != NULL) {
                    if (entry->d_type == DT_DIR) {
                        ESP_LOGI("main", "  📂 %s/", entry->d_name);
                    } else {
                        // Show file with size
                        char filepath[128];
                        snprintf(filepath, sizeof(filepath), "/sdcard/logs/%s", entry->d_name);
                        struct stat st;
                        if (stat(filepath, &st) == 0) {
                            ESP_LOGI("main", "  📄 %s (%ld bytes)", entry->d_name, st.st_size);
                        } else {
                            ESP_LOGI("main", "  📄 %s", entry->d_name);
                        }
                    }
                    log_count++;
                }
                closedir(logs_dir);
                ESP_LOGI("main", "Total: %d log files", log_count);
            } else {
                ESP_LOGI("main", "📁 /sdcard/logs/ (not found)");
            }
            ESP_LOGI("main", "===========================================");
        }
    }
    
    // Create VPIDocument (data model with Observer pattern)
    g_vpi_document = new VPIDocument();
    ESP_LOGI("main", "VPIDocument created");
    
    // Create DataLogger with default config
    DataLogger::Config logger_config = DataLogger::getDefaultConfig();
    logger_config.interval_ms = 1000;          // Log every 1 second
    logger_config.max_file_size_mb = 10;       // Rotate at 10 MB
    logger_config.log_on_state_change = true;  // Force log on state change
    logger_config.base_path = "/sdcard/logs";  // Use subdirectory (root has write restrictions)
    
    g_data_logger = new DataLogger(logger_config);
    ESP_LOGI("main", "DataLogger created (interval=1s, max_size=10MB)");
    
    // Attach DataLogger as observer to VPIDocument (C++ to C++ direct)
    g_vpi_document->attach(g_data_logger);
    ESP_LOGI("main", "DataLogger attached to VPIDocument");
    
    // Use existing g_vpi_document instance (cast to C handle)
    vpi_document_handle_t doc_handle = static_cast<vpi_document_handle_t>(g_vpi_document);
    
    // Set VPIDocument handle for vpi_data_manager (C wrapper)
    vpi_data_manager_set_document(doc_handle);
    ESP_LOGI("main", "VPIDocument handle set for data manager");
    
    // Start logging to SD card (only if SD card mounted successfully)
    if (ret == ESP_OK) {
        // Test SD card write permission first
        ESP_LOGI("main", "===========================================");
        ESP_LOGI("main", "  Testing SD Card Write Capability");
        ESP_LOGI("main", "===========================================");
        
        // Check if path exists and is directory
        struct stat st;
        if (stat("/sdcard", &st) == 0) {
            ESP_LOGI("main", "✓ /sdcard stat OK - mode=0%o, isdir=%d", 
                     st.st_mode & 0777, S_ISDIR(st.st_mode));
        }
        
        // Check access permissions using POSIX access()
        if (access("/sdcard", W_OK) == 0) {
            ESP_LOGI("main", "✓ /sdcard has write permission (access W_OK passed)");
        } else {
            ESP_LOGE("main", "✗ /sdcard write permission denied: %s (errno=%d)", 
                     strerror(errno), errno);
        }
        
        // Try using POSIX open() for more detailed error info
        ESP_LOGI("main", "Trying POSIX open() with O_CREAT|O_WRONLY...");
        int fd = open("/sdcard/test_write.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
        bool sd_writable = false;
        
        if (fd < 0) {
            int err = errno;
            ESP_LOGE("main", "✗ open() failed: %s (errno=%d)", strerror(err), err);
            
            // Try creating a directory first (root directory has write restrictions)
            ESP_LOGI("main", "Trying to create directory /sdcard/logs...");
            if (mkdir("/sdcard/logs", 0777) == 0 || errno == EEXIST) {
                ESP_LOGI("main", "✓ Directory /sdcard/logs ready for CSV logging");
                
                // Try writing in subdirectory
                ESP_LOGI("main", "Trying to write in subdirectory...");
                int fd2 = open("/sdcard/logs/test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
                if (fd2 >= 0) {
                    ESP_LOGI("main", "✓ Subdirectory write succeeded!");
                    const char* test_data = "Subdirectory write OK\n";
                    write(fd2, test_data, strlen(test_data));
                    close(fd2);
                    remove("/sdcard/logs/test.txt");
                    sd_writable = true;
                } else {
                    ESP_LOGE("main", "✗ Subdirectory write also failed: %s", strerror(errno));
                }
            } else {
                ESP_LOGE("main", "✗ Cannot create directory: %s (errno=%d)", strerror(errno), errno);
            }
            
            // Try fopen() as alternative test if still failing
            if (!sd_writable) {
                ESP_LOGI("main", "Trying fopen() as fallback...");
                FILE* test_file = fopen("/sdcard/test_fopen.txt", "w");
                if (test_file == nullptr) {
                    err = errno;
                    ESP_LOGE("main", "✗ fopen() also failed: %s (errno=%d)", strerror(err), err);
                    ESP_LOGE("main", "==========================================");
                    ESP_LOGE("main", "SD CARD WRITE FAILED - Possible causes:");
                    ESP_LOGE("main", "1. Physical write-protect switch ON");
                    ESP_LOGE("main", "2. Filesystem corruption");
                    ESP_LOGE("main", "3. Card is read-only");
                    ESP_LOGE("main", "Solutions:");
                    ESP_LOGE("main", "- Check SD card write-protect switch");
                    ESP_LOGE("main", "- Format card on PC (FAT32)");
                    ESP_LOGE("main", "- Try a different SD card");
                    ESP_LOGE("main", "==========================================");
                    ESP_LOGW("main", "CSV logging will be DISABLED");
                } else {
                    fprintf(test_file, "fopen test OK\n");
                    fclose(test_file);
                    remove("/sdcard/test_fopen.txt");
                    ESP_LOGI("main", "✓ fopen() succeeded!");
                    sd_writable = true;
                }
            }
        } else {
            ESP_LOGI("main", "✓ open() succeeded! fd=%d", fd);
            const char* test_data = "Write test OK\n";
            ssize_t written = write(fd, test_data, strlen(test_data));
            if (written > 0) {
                ESP_LOGI("main", "✓ write() succeeded! %d bytes written", (int)written);
                sd_writable = true;
            } else {
                ESP_LOGE("main", "✗ write() failed: %s (errno=%d)", strerror(errno), errno);
            }
            close(fd);
            remove("/sdcard/test_write.txt");
        }
        
        // Start CSV logging if SD card is writable
        if (sd_writable) {
            // Ensure logs directory exists before starting logger
            mkdir("/sdcard/logs", 0777);  // Create if not exists (ignore error if exists)
            
            ESP_LOGI("main", "SD card write test PASSED - starting CSV logging...");
            if (g_data_logger->startLogging()) {
                ESP_LOGI("main", "✓ CSV logging started: %s", g_data_logger->getCurrentFilename().c_str());
                
                // Sync button state with logging state (must lock LVGL)
                if (g_vpi_hmi_ui && lvgl_port_lock(-1)) {
                    lv_obj_t* log_btn = g_vpi_hmi_ui->getLogButton();
                    if (log_btn) {
                        lv_obj_add_state(log_btn, LV_STATE_CHECKED);
                        g_vpi_hmi_ui->setLoggingState(true);
                        ESP_LOGI("main", "✓ LOG button synced (CHECKED + RED)");
                    }
                    lvgl_port_unlock();
                }
            } else {
                ESP_LOGE("main", "✗ Failed to start CSV logging");
            }
        }
    } else {
        ESP_LOGW("main", "Skipping CSV logging (SD card not mounted)");
    }
    
    ESP_LOGI("main", "===========================================");
    ESP_LOGI("main", "  Data Logger Ready!");
    ESP_LOGI("main", "===========================================");
}

// ========== C Wrapper Functions for VPI Data Manager ==========
// (Allow C code to call C++ UI methods)

extern "C" void ui_vpi_hmi_update_electrical(uint8_t phase, float voltage, float current,
                                             float power, float reactive, float energy)
{
    if (g_vpi_hmi_ui) {
        g_vpi_hmi_ui->updateElectrical(phase, voltage, current, power, reactive, energy);
    }
}

extern "C" void ui_vpi_hmi_update_pressure(float percentage)
{
    ESP_LOGI("main", "C wrapper: update pressure %.2f%%", percentage);
    if (g_vpi_hmi_ui) {
        g_vpi_hmi_ui->updatePressure(percentage);
    } else {
        ESP_LOGW("main", "g_vpi_hmi_ui is NULL!");
    }
}

extern "C" void ui_vpi_hmi_update_status(const char* status)
{
    if (g_vpi_hmi_ui) {
        g_vpi_hmi_ui->updateStatus(status);
    }
}
