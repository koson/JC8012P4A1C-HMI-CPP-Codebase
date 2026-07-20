#include "SystemManager.h"
#include "esp_log.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static const char *TAG = "SystemManager";

#if CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE
static esp_err_t sdmmc_host_init_dummy(void)
{
    return ESP_OK;
}

static esp_err_t sdmmc_host_deinit_dummy(void)
{
    return ESP_OK;
}
#endif

// External SD card handle from BSP
extern "C"
{
    extern sdmmc_card_t *bsp_sdcard;
}

SystemManager &SystemManager::getInstance()
{
    static SystemManager instance;
    return instance;
}

SystemManager::SystemManager()
    : m_display(nullptr), m_touch_indev(nullptr), m_display_initialized(false), m_sd_mounted(false)
{
}

SystemManager::DisplayConfig SystemManager::getDefaultDisplayConfig()
{
    DisplayConfig config;
    // Smaller tiles = shorter SPIRAM hold per tile = fewer DSI underruns during transitions.
    // H_RES * 20 = 16000 pixels per tile (was 100 = 80000). More flush calls but each
    // releases the SPIRAM bus faster, letting DSI refresh without underrun.
    config.buffer_size = BSP_LCD_H_RES * 20;
    config.double_buffer = true; // Pipeline: render tile N+1 while tile N DMAs to DSI FB
    config.use_spiram = true;
    config.use_dma = true;
    config.sw_rotate = true;
    config.rotation = LV_DISPLAY_ROTATION_270; // Landscape mode
    return config;
}

esp_err_t SystemManager::initDisplay(const DisplayConfig *config)
{
    if (m_display_initialized)
    {
        ESP_LOGW(TAG, "Display already initialized");
        return ESP_OK;
    }

    DisplayConfig cfg = config ? *config : getDefaultDisplayConfig();

    ESP_LOGI(TAG, "Initializing display...");
    ESP_LOGI(TAG, "  Buffer size: %lu px (%lu KB)", cfg.buffer_size, (cfg.buffer_size * 2) / 1024);
    ESP_LOGI(TAG, "  Double buffer: %s", cfg.double_buffer ? "Yes" : "No");
    ESP_LOGI(TAG, "  SPIRAM: %s", cfg.use_spiram ? "Yes" : "No");
    ESP_LOGI(TAG, "  DMA: %s", cfg.use_dma ? "Yes" : "No");
    ESP_LOGI(TAG, "  SW Rotation: %s", cfg.sw_rotate ? "Yes" : "No");

    bsp_display_cfg_t bsp_cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = cfg.buffer_size,
        .double_buffer = cfg.double_buffer,
        .flags = {
            .buff_dma = cfg.use_dma,
            .buff_spiram = cfg.use_spiram,
            .sw_rotate = cfg.sw_rotate,
        }};

    lv_display_t *disp = bsp_display_start_with_config(&bsp_cfg);
    if (disp == nullptr)
    {
        ESP_LOGE(TAG, "Failed to start display");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Display started, turning on backlight");
    bsp_display_backlight_on();

    // Get LVGL display object
    m_display = lv_display_get_default();
    if (m_display == nullptr)
    {
        ESP_LOGE(TAG, "Failed to get LVGL display object");
        return ESP_FAIL;
    }

    // Set rotation
    lv_display_set_rotation(m_display, cfg.rotation);
    ESP_LOGI(TAG, "Display rotation set to: %d", cfg.rotation);

    // Check touch input
    m_touch_indev = bsp_display_get_input_dev();
    if (m_touch_indev == nullptr)
    {
        ESP_LOGW(TAG, "⚠️  Touchscreen not available - system will run in display-only mode");
    }
    else
    {
        ESP_LOGI(TAG, "✓ Touchscreen initialized (type=%d)", lv_indev_get_type(m_touch_indev));
    }

    m_display_initialized = true;
    ESP_LOGI(TAG, "✓ Display system initialized");
    return ESP_OK;
}

bool SystemManager::isTouchAvailable()
{
    return m_touch_indev != nullptr;
}

esp_err_t SystemManager::mountSDCard()
{
    if (m_sd_mounted)
    {
        ESP_LOGW(TAG, "SD card already mounted");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Mounting SD card...");
    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_BSP_SD_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 5,
        .allocation_unit_size = 64 * 1024,
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_0;
#if CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE
    host.init = &sdmmc_host_init_dummy;
    host.deinit = &sdmmc_host_deinit_dummy;
#endif
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
    esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
        return ret;
    }
    host.pwr_ctrl_handle = pwr_ctrl_handle;

    const sdmmc_slot_config_t slot_config = {
        .cd = SDMMC_SLOT_NO_CD,
        .wp = SDMMC_SLOT_NO_WP,
        .width = 4,
        .flags = 0,
    };

    ret = esp_vfs_fat_sdmmc_mount(BSP_SD_MOUNT_POINT, &host, &slot_config, &mount_config, &bsp_sdcard);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        return ret;
    }

    m_sd_mounted = true;
    ESP_LOGI(TAG, "✓ SD card mounted successfully");

    // Print SD card info
    if (bsp_sdcard != nullptr)
    {
        sdmmc_card_print_info(stdout, bsp_sdcard);
    }

    // Verify mount point is accessible
    DIR *dir = opendir("/sdcard");
    if (dir == nullptr)
    {
        ESP_LOGE(TAG, "Cannot access /sdcard: %s", strerror(errno));
        m_sd_mounted = false;
        return ESP_FAIL;
    }
    closedir(dir);

    ESP_LOGI(TAG, "SD card ready for use");
    return ESP_OK;
}

bool SystemManager::testSDCardWrite(const char *test_path)
{
    if (!m_sd_mounted)
    {
        ESP_LOGW(TAG, "SD card not mounted");
        return false;
    }

    ESP_LOGI(TAG, "Testing SD card write capability...");

    // Create test directory if it doesn't exist
    struct stat st;
    if (stat(test_path, &st) != 0)
    {
        ESP_LOGI(TAG, "Creating directory: %s", test_path);
        if (mkdir(test_path, 0777) != 0 && errno != EEXIST)
        {
            ESP_LOGE(TAG, "Failed to create directory: %s", strerror(errno));
            return false;
        }
    }

    // Try to write a test file
    char test_file[128];
    snprintf(test_file, sizeof(test_file), "%s/test_write.txt", test_path);

    int fd = open(test_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0)
    {
        ESP_LOGE(TAG, "Failed to create test file: %s", strerror(errno));
        return false;
    }

    const char *test_data = "SD card write test OK\n";
    ssize_t written = write(fd, test_data, strlen(test_data));
    close(fd);
    remove(test_file);

    if (written <= 0)
    {
        ESP_LOGE(TAG, "Failed to write to test file: %s", strerror(errno));
        return false;
    }

    ESP_LOGI(TAG, "✓ SD card write test passed");
    return true;
}

void SystemManager::listSDCardContents(const char *path)
{
    if (!m_sd_mounted)
    {
        ESP_LOGW(TAG, "SD card not mounted");
        return;
    }

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  SD Card Contents: %s", path);
    ESP_LOGI(TAG, "===========================================");

    DIR *dir = opendir(path);
    if (dir == nullptr)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", strerror(errno));
        return;
    }

    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_DIR)
        {
            ESP_LOGI(TAG, "  📂 %s/", entry->d_name);
        }
        else
        {
            // Get file size
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
            struct stat st;
            if (stat(filepath, &st) == 0)
            {
                ESP_LOGI(TAG, "  📄 %s (%ld bytes)", entry->d_name, st.st_size);
            }
            else
            {
                ESP_LOGI(TAG, "  📄 %s", entry->d_name);
            }
        }
        count++;
    }
    closedir(dir);

    ESP_LOGI(TAG, "Total: %d items", count);
    ESP_LOGI(TAG, "===========================================");
}

bool SystemManager::lockDisplay(int timeout_ms)
{
    if (!m_display_initialized)
    {
        ESP_LOGW(TAG, "Display not initialized");
        return false;
    }

    TickType_t timeout = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return bsp_display_lock(timeout);
}

void SystemManager::unlockDisplay()
{
    if (m_display_initialized)
    {
        bsp_display_unlock();
    }
}
