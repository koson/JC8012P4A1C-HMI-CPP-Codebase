#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "IVPIObserver.h"
#include "VPIDataSnapshot.h"
#include <string>
#include <stdio.h>
#include <mutex>

/**
 * @brief CSV Data Logger for VPI System
 * 
 * Implements Observer pattern to automatically log VPI data to CSV files.
 * Features:
 * - Auto-creates timestamped CSV files
 * - Writes header row with column names
 * - Logs data on every update or at specified interval
 * - Thread-safe file operations
 * - Graceful error handling
 */
class DataLogger : public IVPIObserver {
public:
    /**
     * @brief Logging configuration
     */
    struct Config {
        uint32_t interval_ms;      // Minimum interval between logs (0 = every update)
        size_t max_file_size_mb;   // Max file size before rotation (0 = no limit)
        bool log_on_state_change;  // Force log on VPI state change
        const char* base_path;     // Base path for log files (e.g., "/sdcard")
    };
    
    DataLogger(const Config& config = getDefaultConfig());
    ~DataLogger();
    
    // ========== Logging Control ==========
    /**
     * @brief Start logging to a new CSV file
     * @param filename Optional custom filename (auto-generated if nullptr)
     * @return true if file created successfully
     */
    bool startLogging(const char* filename = nullptr);
    
    /**
     * @brief Stop logging and close file
     */
    void stopLogging();
    
    /**
     * @brief Check if currently logging
     * @return true if logging active
     */
    bool isLogging() const { return log_file_ != nullptr; }
    
    /**
     * @brief Get current log filename
     * @return Filename or empty string if not logging
     */
    std::string getCurrentFilename() const { return current_filename_; }
    
    // ========== Observer Interface ==========
    /**
     * @brief Called when VPI data is updated
     * @param snapshot Current VPI data
     */
    void onDataUpdate(const vpi_data_snapshot_t& snapshot) override;
    
    // ========== Statistics ==========
    /**
     * @brief Get number of records logged in current session
     */
    uint32_t getRecordCount() const { return record_count_; }
    
    /**
     * @brief Get current log file size in bytes
     */
    size_t getFileSize() const;
    
    /**
     * @brief Get default configuration
     */
    static Config getDefaultConfig();
    
private:
    /**
     * @brief Generate timestamped filename
     * @return Filename like "vpi_log_20260421_123045.csv"
     */
    std::string generateFilename() const;
    
    /**
     * @brief Write CSV header row
     */
    void writeHeader();
    
    /**
     * @brief Write data row to CSV
     * @param snapshot VPI data snapshot
     */
    void writeDataRow(const vpi_data_snapshot_t& snapshot);
    
    /**
     * @brief Check if should log (based on interval and state change)
     * @param snapshot Current data
     * @return true if should write to file
     */
    bool shouldLog(const vpi_data_snapshot_t& snapshot);
    
    Config config_;
    FILE* log_file_;
    std::string current_filename_;
    uint32_t record_count_;
    uint32_t last_log_time_ms_;
    vpi_state_t last_logged_state_;
    bool pm2230_ready_;              // PM2230 has sent valid data (voltage > 0)
    uint32_t start_logging_time_ms_; // Time when logging started
    mutable std::mutex mutex_;
};

#endif // DATA_LOGGER_H
