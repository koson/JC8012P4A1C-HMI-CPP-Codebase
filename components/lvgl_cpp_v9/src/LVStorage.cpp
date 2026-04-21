#include "LVStorage.hpp"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"  // For ESP32-P4 SD power control
#include "ff.h"  // FATFS definitions
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

static const char* TAG = "LVStorage";

LVStorage* LVStorage::s_instance = nullptr;

LVStorage::LVStorage()
    : m_storageType(SD_CARD)
    , m_mountPoint("/sdcard")
    , m_mounted(false)
    , m_cardHandle(nullptr)
{
}

LVStorage::~LVStorage()
{
    if (m_mounted) {
        unmount();
    }
}

LVStorage& LVStorage::getInstance()
{
    if (!s_instance) {
        s_instance = new LVStorage();
    }
    return *s_instance;
}

void LVStorage::destroyInstance()
{
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

bool LVStorage::mount(StorageType type, const char* mountPoint)
{
    if (m_mounted) {
        ESP_LOGW(TAG, "Storage already mounted");
        return false;
    }
    
    m_storageType = type;
    m_mountPoint = mountPoint;
    
    ESP_LOGI(TAG, "Mounting %s storage at %s", 
        type == SD_CARD ? "SD_CARD" : "SPIFFS", mountPoint);
    
    esp_err_t ret;
    
    switch (type) {
        case SD_CARD: {
            // SD card configuration
            esp_vfs_fat_sdmmc_mount_config_t mount_config = {
                .format_if_mount_failed = false,
                .max_files = 5,
                .allocation_unit_size = 16 * 1024,
                .disk_status_check_enable = false,
                .use_one_fat = false
            };
            
            sdmmc_card_t* card;
            
            // Configure SD card power control using on-chip LDO (ESP32-P4)
            sd_pwr_ctrl_ldo_config_t ldo_config = {
                .ldo_chan_id = 4,  // LDO channel 4 for SD card
            };
            sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
            ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to create SD card LDO power control: %s", esp_err_to_name(ret));
                return false;
            }
            
            // Configure SDMMC host for ESP32-P4
            sdmmc_host_t host = SDMMC_HOST_DEFAULT();
            host.slot = SDMMC_HOST_SLOT_0;  // Explicitly use slot 0
            host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;  // Use high speed like BSP
            host.pwr_ctrl_handle = pwr_ctrl_handle;  // IMPORTANT: Set power control handle
            
            // SD card slot configuration
            sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
            slot_config.width = 4;  // Use 4-bit mode like BSP
            slot_config.cd = SDMMC_SLOT_NO_CD;  // No card detect pin
            slot_config.wp = SDMMC_SLOT_NO_WP;  // No write protect pin
            slot_config.flags = 0;  // Default flags
            
            ret = esp_vfs_fat_sdmmc_mount(mountPoint, &host, &slot_config, &mount_config, &card);
            
            if (ret == ESP_OK) {
                m_cardHandle = (void*)card;
                m_mounted = true;
                
                // Print card info
                ESP_LOGI(TAG, "SD card mounted successfully");
                ESP_LOGI(TAG, "   Name: %s", card->cid.name);
                ESP_LOGI(TAG, "   Type: %s", (card->ocr & (1 << 30)) ? "SDHC/SDXC" : "SDSC"); // Bit 30 = CCS
                ESP_LOGI(TAG, "   Speed: %s", (card->csd.tr_speed > 25000000) ? "high speed" : "default speed");
                ESP_LOGI(TAG, "   Size: %llu MB", ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
            } else {
                ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
                return false;
            }
            break;
        }
        
        case SPIFFS: {
            // SPIFFS support will be added later if needed
            ESP_LOGE(TAG, "SPIFFS support not yet implemented");
            ret = ESP_ERR_NOT_SUPPORTED;
            break;
        }
        
        default:
            ESP_LOGE(TAG, "Unsupported storage type");
            return false;
    }
    
    return true;
}

bool LVStorage::unmount()
{
    if (!m_mounted) {
        return true;
    }
    
    ESP_LOGI(TAG, "Unmounting storage");
    
    esp_err_t ret;
    
    switch (m_storageType) {
        case SD_CARD:
            ret = esp_vfs_fat_sdcard_unmount(m_mountPoint.c_str(), (sdmmc_card_t*)m_cardHandle);
            if (ret == ESP_OK) {
                m_mounted = false;
                m_cardHandle = nullptr;
                ESP_LOGI(TAG, "SD card unmounted");
            }
            break;
            
        case SPIFFS:
            // SPIFFS support not yet implemented
            ret = ESP_ERR_NOT_SUPPORTED;
            break;
            
        default:
            return false;
    }
    
    return (ret == ESP_OK);
}

bool LVStorage::fileExists(const char* path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

bool LVStorage::dirExists(const char* path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

size_t LVStorage::getFileSize(const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

bool LVStorage::readFile(const char* path, std::vector<uint8_t>& data)
{
    FILE* f = fopen(path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for reading: %s", path);
        return false;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read data
    data.resize(size);
    size_t read = fread(data.data(), 1, size, f);
    fclose(f);
    
    if (read != size) {
        ESP_LOGE(TAG, "Failed to read file completely: %s", path);
        return false;
    }
    
    ESP_LOGD(TAG, "Read %d bytes from %s", size, path);
    return true;
}

bool LVStorage::writeFile(const char* path, const uint8_t* data, size_t size)
{
    FILE* f = fopen(path, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return false;
    }
    
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    
    if (written != size) {
        ESP_LOGE(TAG, "Failed to write file completely: %s", path);
        return false;
    }
    
    ESP_LOGD(TAG, "Wrote %d bytes to %s", size, path);
    return true;
}

bool LVStorage::appendFile(const char* path, const uint8_t* data, size_t size)
{
    FILE* f = fopen(path, "ab");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for appending: %s", path);
        return false;
    }
    
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    
    if (written != size) {
        ESP_LOGE(TAG, "Failed to append to file: %s", path);
        return false;
    }
    
    ESP_LOGD(TAG, "Appended %d bytes to %s", size, path);
    return true;
}

bool LVStorage::deleteFile(const char* path)
{
    if (unlink(path) == 0) {
        ESP_LOGD(TAG, "Deleted file: %s", path);
        return true;
    }
    ESP_LOGE(TAG, "Failed to delete file: %s", path);
    return false;
}

bool LVStorage::renameFile(const char* oldPath, const char* newPath)
{
    if (rename(oldPath, newPath) == 0) {
        ESP_LOGD(TAG, "Renamed %s to %s", oldPath, newPath);
        return true;
    }
    ESP_LOGE(TAG, "Failed to rename file");
    return false;
}

bool LVStorage::readTextFile(const char* path, std::string& text)
{
    std::vector<uint8_t> data;
    if (!readFile(path, data)) {
        return false;
    }
    
    text.assign(data.begin(), data.end());
    return true;
}

bool LVStorage::writeTextFile(const char* path, const std::string& text)
{
    return writeFile(path, (const uint8_t*)text.c_str(), text.length());
}

bool LVStorage::readTextLines(const char* path, std::vector<std::string>& lines)
{
    std::string content;
    if (!readTextFile(path, content)) {
        return false;
    }
    
    std::istringstream stream(content);
    std::string line;
    lines.clear();
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    return true;
}

bool LVStorage::createDir(const char* path)
{
    if (mkdir(path, 0775) == 0) {
        ESP_LOGD(TAG, "Created directory: %s", path);
        return true;
    }
    
    if (errno == EEXIST) {
        return true;  // Already exists
    }
    
    ESP_LOGE(TAG, "Failed to create directory: %s", path);
    return false;
}

bool LVStorage::removeDir(const char* path)
{
    if (rmdir(path) == 0) {
        ESP_LOGD(TAG, "Removed directory: %s", path);
        return true;
    }
    ESP_LOGE(TAG, "Failed to remove directory: %s", path);
    return false;
}

std::vector<std::string> LVStorage::listDir(const char* path, bool filesOnly)
{
    std::vector<std::string> items;
    
    DIR* dir = opendir(path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", path);
        return items;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        if (filesOnly && entry->d_type == DT_DIR) {
            continue;  // Skip directories if filesOnly
        }
        
        items.push_back(entry->d_name);
    }
    
    closedir(dir);
    
    ESP_LOGD(TAG, "Listed %d items in %s", items.size(), path);
    return items;
}

size_t LVStorage::getTotalSpace()
{
    if (!m_mounted) {
        return 0;
    }
    
    if (m_storageType == SD_CARD && m_cardHandle) {
        sdmmc_card_t* card = (sdmmc_card_t*)m_cardHandle;
        return ((uint64_t)card->csd.capacity) * card->csd.sector_size;
    } else if (m_storageType == SPIFFS) {
        // SPIFFS support not yet implemented
        return 0;
    }
    
    return 0;
}

size_t LVStorage::getFreeSpace()
{
    if (!m_mounted) {
        return 0;
    }
    
    if (m_storageType == SPIFFS) {
        // SPIFFS support not yet implemented
        return 0;
    }
    
    // For SD card, calculate from total space (not ideal but works)
    // In production, you'd use statvfs() POSIX API instead of f_getfree
    size_t total = getTotalSpace();
    // Return approximate free space (80% of total as rough estimate)
    // TODO: Implement proper statvfs() call for accurate free space
    return (total * 80) / 100;
}
