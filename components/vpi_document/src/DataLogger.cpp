#include "DataLogger.h"
#include "esp_log.h"
#include <cstring>
#include <ctime>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>  // for fsync(), fileno()
#include <errno.h>

static const char* TAG = "DataLogger";

// ========== DataLogger Implementation ==========

DataLogger::Config DataLogger::getDefaultConfig() {
    return {
        .interval_ms = 1000,           // Log every 1 second
        .max_file_size_mb = 10,        // Rotate at 10 MB
        .log_on_state_change = true,   // Always log on state change
        .base_path = "/sdcard"         // SD card mount point
    };
}

DataLogger::DataLogger(const Config& config)
    : config_(config)
    , log_file_(nullptr)
    , record_count_(0)
    , last_log_time_ms_(0)
    , last_logged_state_(VPI_STATE_IDLE)
    , pm2230_ready_(false)
    , start_logging_time_ms_(0)
{
    ESP_LOGI(TAG, "DataLogger created (interval=%lu ms, max_size=%lu MB)", 
             config_.interval_ms, config_.max_file_size_mb);
}

DataLogger::~DataLogger() {
    stopLogging();
    ESP_LOGI(TAG, "DataLogger destroyed");
}

bool DataLogger::startLogging(const char* filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Stop current logging if active
    if (log_file_ != nullptr) {
        stopLogging();
    }
    
    // Generate filename if not provided
    if (filename == nullptr || strlen(filename) == 0) {
        current_filename_ = generateFilename();
    } else {
        current_filename_ = std::string(config_.base_path) + "/" + filename;
    }
    
    ESP_LOGI(TAG, "Attempting to create log file: %s", current_filename_.c_str());
    ESP_LOGI(TAG, "Filename length: %zu characters", current_filename_.length());
    
    // Open file for writing
    log_file_ = fopen(current_filename_.c_str(), "w");
    if (log_file_ == nullptr) {
        int err = errno;
        ESP_LOGE(TAG, "Failed to open log file: %s", current_filename_.c_str());
        ESP_LOGE(TAG, "Error: %s (errno=%d)", strerror(err), err);
        
        // Try with a simple short filename as fallback
        std::string simple_name = std::string(config_.base_path) + "/data.csv";
        ESP_LOGW(TAG, "Trying fallback filename: %s", simple_name.c_str());
        log_file_ = fopen(simple_name.c_str(), "w");
        
        if (log_file_ != nullptr) {
            ESP_LOGI(TAG, "✓ Fallback filename worked! Using: %s", simple_name.c_str());
            current_filename_ = simple_name;
        } else {
            err = errno;
            ESP_LOGE(TAG, "✗ Fallback also failed: %s (errno=%d)", strerror(err), err);
            current_filename_.clear();
            return false;
        }
    } else {
        ESP_LOGI(TAG, "✓ Log file opened successfully");
    }
    
    // Write CSV header
    writeHeader();
    
    // Reset counters and flags
    record_count_ = 0;
    last_log_time_ms_ = 0;
    pm2230_ready_ = false;
    
    // Record start time
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    start_logging_time_ms_ = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    
    ESP_LOGI(TAG, "Logging started: %s (waiting for PM2230 data...)", current_filename_.c_str());
    return true;
}

void DataLogger::stopLogging() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (log_file_ != nullptr) {
        // Final flush and sync before closing
        fflush(log_file_);
        fsync(fileno(log_file_));
        fclose(log_file_);
        log_file_ = nullptr;
        
        ESP_LOGI(TAG, "Logging stopped: %s (%lu records, flushed to SD)", 
                 current_filename_.c_str(), record_count_);
        
        current_filename_.clear();
        record_count_ = 0;
    }
}

void DataLogger::onDataUpdate(const vpi_data_snapshot_t& snapshot) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if logging is active
    if (log_file_ == nullptr) {
        ESP_LOGW(TAG, "onDataUpdate called but log_file is NULL!");
        return;
    }
    
    // Detect PM2230 ready (has valid voltage data)
    if (!pm2230_ready_ && (snapshot.pm2230.voltage_a > 10.0f || 
                            snapshot.pm2230.voltage_b > 10.0f || 
                            snapshot.pm2230.voltage_c > 10.0f)) {
        pm2230_ready_ = true;
        ESP_LOGI(TAG, "✓ PM2230 ready - starting CSV logging (V=%.1f/%.1f/%.1f)",
                 snapshot.pm2230.voltage_a, snapshot.pm2230.voltage_b, snapshot.pm2230.voltage_c);
    }
    
    // Check if should log
    if (!shouldLog(snapshot)) {
        ESP_LOGD(TAG, "shouldLog returned false, skipping");
        return;
    }
    
    ESP_LOGI(TAG, "Writing: pressure=%.2f mA, PM2230: V=%.1f/%.1f/%.1f V, P=%u W", 
             snapshot.pressure.current_ma,
             snapshot.pm2230.voltage_a, snapshot.pm2230.voltage_b, snapshot.pm2230.voltage_c,
             snapshot.pm2230.power);
    
    // Write data row
    writeDataRow(snapshot);
    
    // Update state tracking
    last_logged_state_ = snapshot.state;
    
    // Check file size for rotation
    if (config_.max_file_size_mb > 0) {
        size_t file_size = getFileSize();
        size_t max_size = config_.max_file_size_mb * 1024 * 1024;
        
        if (file_size >= max_size) {
            ESP_LOGW(TAG, "Log file reached max size (%lu bytes), rotating...", file_size);
            
            // Close current file
            fflush(log_file_);
            fclose(log_file_);
            log_file_ = nullptr;
            
            // Start new file
            current_filename_ = generateFilename();
            log_file_ = fopen(current_filename_.c_str(), "w");
            
            if (log_file_ != nullptr) {
                writeHeader();
                record_count_ = 0;
                ESP_LOGI(TAG, "Log rotation complete: %s", current_filename_.c_str());
            } else {
                ESP_LOGE(TAG, "Failed to create rotated log file");
            }
        }
    }
}

size_t DataLogger::getFileSize() const {
    if (log_file_ == nullptr) {
        return 0;
    }
    
    struct stat st;
    if (stat(current_filename_.c_str(), &st) == 0) {
        return st.st_size;
    }
    
    return 0;
}

std::string DataLogger::generateFilename() const {
    // Get current time
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo);
    
    // Format: vpi_log_YYYYMMDD_HHMMSS.csv
    char filename[128];
    snprintf(filename, sizeof(filename), "%s/vpi_log_%04d%02d%02d_%02d%02d%02d.csv",
             config_.base_path,
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
    
    return std::string(filename);
}

void DataLogger::writeHeader() {
    if (log_file_ == nullptr) {
        return;
    }
    
    fprintf(log_file_, "timestamp,state,v_a,v_b,v_c,i_a,i_b,i_c,power,pressure_ma,pressure_bar,quality\n");
    fflush(log_file_);
    fsync(fileno(log_file_));  // Force write to SD card
    ESP_LOGI(TAG, "CSV header written and flushed to SD card");
}

void DataLogger::writeDataRow(const vpi_data_snapshot_t& snapshot) {
    if (log_file_ == nullptr) {
        return;
    }
    
    // Format timestamp as ISO 8601
    struct tm timeinfo;
    localtime_r(&snapshot.timestamp, &timeinfo);
    char timestamp_str[32];
    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    // Write CSV row
    fprintf(log_file_, "%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%.2f,%.3f,%u\n",
            timestamp_str,
            vpi_state_to_string(snapshot.state),
            snapshot.pm2230.voltage_a,
            snapshot.pm2230.voltage_b,
            snapshot.pm2230.voltage_c,
            snapshot.pm2230.current_a,
            snapshot.pm2230.current_b,
            snapshot.pm2230.current_c,
            snapshot.pm2230.power,
            snapshot.pressure.current_ma,
            snapshot.pressure.pressure_bar,
            snapshot.pressure.quality);
    
    // Flush to SD card immediately
    fflush(log_file_);
    
    record_count_++;
    
    // Force sync to SD card every record (aggressive flush)
    fsync(fileno(log_file_));
    
    // Log file size after write
    size_t file_size = getFileSize();
    ESP_LOGI(TAG, "Record #%lu written, file size: %zu bytes (PM2230: V=%.1f/%.1f/%.1f, P=%u W)",
             record_count_, file_size,
             snapshot.pm2230.voltage_a, snapshot.pm2230.voltage_b, snapshot.pm2230.voltage_c,
             snapshot.pm2230.power);
    
    // Update last log time
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    last_log_time_ms_ = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

bool DataLogger::shouldLog(const vpi_data_snapshot_t& snapshot) {
    // Wait for PM2230 to be ready before logging
    // Exception: after 10 seconds timeout, start logging anyway (pressure-only mode)
    if (!pm2230_ready_) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        uint32_t current_time_ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        uint32_t elapsed_ms = current_time_ms - start_logging_time_ms_;
        
        if (elapsed_ms < 10000) {  // Wait up to 10 seconds
            return false;  // PM2230 not ready, skip logging
        } else {
            // Timeout - start logging without PM2230 data
            ESP_LOGW(TAG, "PM2230 timeout (10s) - logging pressure-only data");
            pm2230_ready_ = true;  // Don't show this warning again
        }
    }
    
    // Always log on state change if configured
    if (config_.log_on_state_change && snapshot.state != last_logged_state_) {
        ESP_LOGI(TAG, "Logging state change: %s -> %s",
                 vpi_state_to_string(last_logged_state_),
                 vpi_state_to_string(snapshot.state));
        return true;
    }
    
    // Check interval
    if (config_.interval_ms > 0) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        uint32_t current_time_ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        
        if ((current_time_ms - last_log_time_ms_) < config_.interval_ms) {
            return false;  // Too soon since last log
        }
    }
    
    return true;  // Log by default
}
