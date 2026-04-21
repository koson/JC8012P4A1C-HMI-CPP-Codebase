#pragma once

#include "LVObject.hpp"
#include <string>
#include <vector>
#include <cstdint>

/**
 * @file LVStorage.hpp
 * @brief File system wrapper for SD card, SPIFFS, and LittleFS (similar to MFC CFile)
 * 
 * LVStorage provides unified interface for different storage types:
 * - SD Card (via ESP-IDF SD card driver)
 * - SPIFFS (SPI Flash File System)
 * - LittleFS (Little File System)
 * - FatFS
 * 
 * Features:
 * - Singleton pattern for easy access
 * - Automatic mount/unmount
 * - File read/write operations
 * - Directory operations
 * - Storage info (total/free space)
 * - Text file helpers
 * 
 * Example:
 * @code
 * auto& storage = LVStorage::getInstance();
 * 
 * // Mount SD card
 * if (storage.mount(LVStorage::SD_CARD, "/sdcard")) {
 *     // Write text file
 *     storage.writeTextFile("/sdcard/config.txt", "brightness=50\n");
 *     
 *     // Read text file
 *     std::string content;
 *     storage.readTextFile("/sdcard/config.txt", content);
 *     
 *     // Check space
 *     size_t free = storage.getFreeSpace();
 * }
 * @endcode
 */
class LVStorage : public LVObject {
public:
    /**
     * @brief Storage type enumeration
     */
    enum StorageType {
        SD_CARD,      ///< SD card via SPI/SDMMC
        SPIFFS,       ///< SPI Flash File System
        LITTLEFS,     ///< Little File System
        FATFS         ///< FAT File System
    };
    
private:
    static LVStorage* s_instance;
    
    StorageType m_storageType;
    std::string m_mountPoint;
    bool m_mounted;
    void* m_cardHandle;  // SD card handle (if using SD card)
    
    LVStorage();
    ~LVStorage() override;
    
    // Prevent copying
    LVStorage(const LVStorage&) = delete;
    LVStorage& operator=(const LVStorage&) = delete;
    
public:
    /**
     * @brief Get singleton instance
     * @return LVStorage& Reference to singleton instance
     */
    static LVStorage& getInstance();
    
    /**
     * @brief Destroy singleton instance
     */
    static void destroyInstance();
    
    // ==================== Initialization ====================
    
    /**
     * @brief Mount storage device
     * 
     * @param type Storage type to mount
     * @param mountPoint Mount point path (e.g., "/sdcard")
     * @return true if mounted successfully
     * @return false on error
     */
    bool mount(StorageType type, const char* mountPoint = "/sdcard");
    
    /**
     * @brief Unmount storage device
     * @return true if unmounted successfully
     */
    bool unmount();
    
    /**
     * @brief Check if storage is ready
     * @return true if mounted and ready
     */
    bool isReady() const { return m_mounted; }
    
    /**
     * @brief Get mount point path
     * @return const char* Mount point string
     */
    const char* getMountPoint() const { return m_mountPoint.c_str(); }
    
    // ==================== File Operations ====================
    
    /**
     * @brief Check if file exists
     * @param path File path
     * @return true if file exists
     */
    bool fileExists(const char* path);
    
    /**
     * @brief Check if directory exists
     * @param path Directory path
     * @return true if directory exists
     */
    bool dirExists(const char* path);
    
    /**
     * @brief Get file size
     * @param path File path
     * @return size_t File size in bytes, 0 if file doesn't exist
     */
    size_t getFileSize(const char* path);
    
    /**
     * @brief Read entire file into memory
     * 
     * @param path File path
     * @param data Output vector to store file data
     * @return true if read successfully
     */
    bool readFile(const char* path, std::vector<uint8_t>& data);
    
    /**
     * @brief Write data to file (overwrites existing file)
     * 
     * @param path File path
     * @param data Data to write
     * @param size Data size in bytes
     * @return true if written successfully
     */
    bool writeFile(const char* path, const uint8_t* data, size_t size);
    
    /**
     * @brief Append data to file
     * 
     * @param path File path
     * @param data Data to append
     * @param size Data size in bytes
     * @return true if appended successfully
     */
    bool appendFile(const char* path, const uint8_t* data, size_t size);
    
    /**
     * @brief Delete file
     * @param path File path
     * @return true if deleted successfully
     */
    bool deleteFile(const char* path);
    
    /**
     * @brief Rename/move file
     * @param oldPath Old file path
     * @param newPath New file path
     * @return true if renamed successfully
     */
    bool renameFile(const char* oldPath, const char* newPath);
    
    // ==================== Text File Helpers ====================
    
    /**
     * @brief Read entire text file as string
     * 
     * @param path File path
     * @param text Output string
     * @return true if read successfully
     */
    bool readTextFile(const char* path, std::string& text);
    
    /**
     * @brief Write string to text file
     * 
     * @param path File path
     * @param text Text to write
     * @return true if written successfully
     */
    bool writeTextFile(const char* path, const std::string& text);
    
    /**
     * @brief Read text file line by line
     * 
     * @param path File path
     * @param lines Output vector of lines
     * @return true if read successfully
     */
    bool readTextLines(const char* path, std::vector<std::string>& lines);
    
    // ==================== Directory Operations ====================
    
    /**
     * @brief Create directory (including parent directories)
     * @param path Directory path
     * @return true if created successfully
     */
    bool createDir(const char* path);
    
    /**
     * @brief Remove empty directory
     * @param path Directory path
     * @return true if removed successfully
     */
    bool removeDir(const char* path);
    
    /**
     * @brief List directory contents
     * 
     * @param path Directory path
     * @param filesOnly If true, only list files (not directories)
     * @return std::vector<std::string> List of file/directory names
     */
    std::vector<std::string> listDir(const char* path, bool filesOnly = false);
    
    // ==================== Storage Info ====================
    
    /**
     * @brief Get total storage space
     * @return size_t Total space in bytes
     */
    size_t getTotalSpace();
    
    /**
     * @brief Get free storage space
     * @return size_t Free space in bytes
     */
    size_t getFreeSpace();
    
    /**
     * @brief Get used storage space
     * @return size_t Used space in bytes
     */
    size_t getUsedSpace() {
        return getTotalSpace() - getFreeSpace();
    }
    
    // ==================== RTTI ====================
    
    DECLARE_LVOBJECT_CLASS(LVStorage, LVObject)
};
