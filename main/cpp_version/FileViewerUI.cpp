#include "FileViewerUI.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

static const char *TAG = "FileViewerUI";
static const char *UPLOAD_DIR = "/sdcard/WORKSHOP";

// Constructor
FileViewerUI::FileViewerUI()
    : m_parent(nullptr), m_fileManagerContainer(nullptr), m_renderScreen(nullptr),
      m_btnBack(nullptr), m_labelIP(nullptr), m_labelStatus(nullptr),
      m_fileList(nullptr), m_btnRefresh(nullptr), m_btnRender(nullptr),
      m_btnClear(nullptr), m_canvasContainer(nullptr),
      m_canvas(nullptr), m_canvasBuffer(nullptr), m_selectedIndex(-1)
{
}

// Destructor
FileViewerUI::~FileViewerUI()
{
    if (m_renderer)
    {
        m_renderer.reset();
    }
    if (m_canvas)
    {
        delete m_canvas;
        m_canvas = nullptr;
    }
    if (m_canvasBuffer)
    {
        heap_caps_free(m_canvasBuffer);
        m_canvasBuffer = nullptr;
    }
}

// Create UI
void FileViewerUI::create(lv_obj_t *parent)
{
    m_parent = parent; // Keep for initialization check

    // ===== File Manager Screen — fixed 1280×800, NO scroll =====
    m_fileManagerContainer = lv_obj_create(NULL); // NULL = top-level screen
    lv_obj_set_size(m_fileManagerContainer, 1280, 800);
    lv_obj_clear_flag(m_fileManagerContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(m_fileManagerContainer, 0, 0);
    lv_obj_set_style_bg_color(m_fileManagerContainer, lv_color_hex(0xF0F2F5), 0);

    lv_obj_t *mainContainer = m_fileManagerContainer;

    // ===== Header Section — fixed 80px tall =====
    lv_obj_t *headerContainer = lv_obj_create(mainContainer);
    lv_obj_set_pos(headerContainer, 0, 0);
    lv_obj_set_size(headerContainer, 1280, 80);
    lv_obj_clear_flag(headerContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(headerContainer, 12, 0);
    lv_obj_set_style_radius(headerContainer, 0, 0);
    lv_obj_set_style_border_width(headerContainer, 0, 0);
    lv_obj_set_style_bg_color(headerContainer, lv_color_hex(0x667eea), 0);

    // Title
    lv_obj_t *titleLabel = lv_label_create(headerContainer);
    lv_label_set_text(titleLabel, "LabBuddy File Manager");
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    // IP Label
    m_labelIP = lv_label_create(headerContainer);
    lv_label_set_text(m_labelIP, "IP: Connecting...");
    lv_obj_set_style_text_font(m_labelIP, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(m_labelIP, lv_color_white(), 0);
    lv_obj_align(m_labelIP, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // Status Label (right side of header)
    m_labelStatus = lv_label_create(headerContainer);
    lv_label_set_text(m_labelStatus, "Ready");
    lv_obj_set_style_text_font(m_labelStatus, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(m_labelStatus, lv_color_white(), 0);
    lv_obj_align(m_labelStatus, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    // ===== Control Buttons =====
    // ===== Button Bar — fixed at y=80, height=60 =====
    lv_obj_t *btnContainer = lv_obj_create(mainContainer);
    lv_obj_set_pos(btnContainer, 0, 80);
    lv_obj_set_size(btnContainer, 1280, 60);
    lv_obj_clear_flag(btnContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(btnContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btnContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btnContainer, 8, 0);
    lv_obj_set_style_pad_column(btnContainer, 8, 0);
    lv_obj_set_style_radius(btnContainer, 0, 0);
    lv_obj_set_style_border_width(btnContainer, 0, 0);
    lv_obj_set_style_bg_color(btnContainer, lv_color_hex(0xEEEEEE), 0);

    // Refresh Button
    m_btnRefresh = lv_button_create(btnContainer);
    lv_obj_set_size(m_btnRefresh, 120, 44);
    lv_obj_add_event_cb(m_btnRefresh, btnRefreshClicked, LV_EVENT_CLICKED, this);
    lv_obj_t *labelRefresh = lv_label_create(m_btnRefresh);
    lv_label_set_text(labelRefresh, "Refresh");
    lv_obj_set_style_text_font(labelRefresh, &lv_font_montserrat_14, 0);
    lv_obj_center(labelRefresh);

    // Render Button
    m_btnRender = lv_button_create(btnContainer);
    lv_obj_set_size(m_btnRender, 120, 44);
    lv_obj_add_event_cb(m_btnRender, btnRenderClicked, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(m_btnRender, lv_color_hex(0x4CAF50), 0);
    lv_obj_t *labelRender = lv_label_create(m_btnRender);
    lv_label_set_text(labelRender, "Render");
    lv_obj_set_style_text_font(labelRender, &lv_font_montserrat_14, 0);
    lv_obj_center(labelRender);

    // Clear Button
    m_btnClear = lv_button_create(btnContainer);
    lv_obj_set_size(m_btnClear, 120, 44);
    lv_obj_add_event_cb(m_btnClear, btnClearClicked, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(m_btnClear, lv_color_hex(0xf44336), 0);
    lv_obj_t *labelClear = lv_label_create(m_btnClear);
    lv_label_set_text(labelClear, "Clear");
    lv_obj_set_style_text_font(labelClear, &lv_font_montserrat_14, 0);
    lv_obj_center(labelClear);

    // ===== File List — fixed at y=140, fills remaining screen height =====
    // Remaining: 800 - 80(header) - 60(buttons) = 660px
    lv_obj_t *listContainer = lv_obj_create(mainContainer);
    lv_obj_set_pos(listContainer, 0, 140);
    lv_obj_set_size(listContainer, 1280, 660);
    lv_obj_clear_flag(listContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(listContainer, 0, 0);
    lv_obj_set_style_radius(listContainer, 0, 0);
    lv_obj_set_style_border_width(listContainer, 0, 0);
    lv_obj_set_style_bg_color(listContainer, lv_color_hex(0xF0F2F5), 0);

    m_fileList = lv_list_create(listContainer);
    lv_obj_set_size(m_fileList, 1280, 660);
    lv_obj_set_pos(m_fileList, 0, 0);
    // File list itself can scroll internally (within its fixed 660px container)

    // ===== Canvas Container (stub, not used — canvas goes on render screen) =====
    m_canvasContainer = lv_obj_create(mainContainer);
    lv_obj_set_size(m_canvasContainer, 1, 1);
    lv_obj_add_flag(m_canvasContainer, LV_OBJ_FLAG_HIDDEN);

    // ===== Render Screen (separate top-level LVGL screen) =====
    m_renderScreen = lv_obj_create(NULL); // NULL = top-level screen
    lv_obj_set_style_bg_color(m_renderScreen, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_pad_all(m_renderScreen, 0, 0);
    lv_obj_clear_flag(m_renderScreen, LV_OBJ_FLAG_SCROLLABLE);

    // Back button (top-left corner of render screen)
    m_btnBack = lv_button_create(m_renderScreen);
    lv_obj_set_size(m_btnBack, 90, 40);
    lv_obj_align(m_btnBack, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_style_bg_color(m_btnBack, lv_color_hex(0x444466), 0);
    lv_obj_set_style_bg_opa(m_btnBack, LV_OPA_80, 0);
    lv_obj_add_event_cb(m_btnBack, btnBackClicked, LV_EVENT_CLICKED, this);
    lv_obj_t *labelBack = lv_label_create(m_btnBack);
    lv_label_set_text(labelBack, "← Back");
    lv_obj_set_style_text_font(labelBack, &lv_font_montserrat_14, 0);
    lv_obj_center(labelBack);

    // Initial file scan and load file manager screen
    refreshFileList();
    lv_screen_load(m_fileManagerContainer);

    // Log actual display + container sizes for layout debugging
    lv_display_t *disp = lv_display_get_default();
    ESP_LOGI(TAG, "=== LAYOUT DEBUG ===");
    ESP_LOGI(TAG, "Display: %d x %d", lv_display_get_horizontal_resolution(disp), lv_display_get_vertical_resolution(disp));
    ESP_LOGI(TAG, "Screen:  pos(%d,%d) size(%d x %d)",
             lv_obj_get_x(m_fileManagerContainer), lv_obj_get_y(m_fileManagerContainer),
             lv_obj_get_width(m_fileManagerContainer), lv_obj_get_height(m_fileManagerContainer));
    ESP_LOGI(TAG, "Header:  pos(%d,%d) size(%d x %d)",
             lv_obj_get_x(headerContainer), lv_obj_get_y(headerContainer),
             lv_obj_get_width(headerContainer), lv_obj_get_height(headerContainer));
    ESP_LOGI(TAG, "BtnBar:  pos(%d,%d) size(%d x %d)",
             lv_obj_get_x(btnContainer), lv_obj_get_y(btnContainer),
             lv_obj_get_width(btnContainer), lv_obj_get_height(btnContainer));
    ESP_LOGI(TAG, "FileList: pos(%d,%d) size(%d x %d)",
             lv_obj_get_x(listContainer), lv_obj_get_y(listContainer),
             lv_obj_get_width(listContainer), lv_obj_get_height(listContainer));
    ESP_LOGI(TAG, "===================");

    ESP_LOGI(TAG, "FileViewerUI created");
}

// Update IP address
void FileViewerUI::updateIP(const char *ip)
{
    if (m_labelIP)
    {
        lv_label_set_text_fmt(m_labelIP, "IP: %s", ip);
        ESP_LOGI(TAG, "IP updated: %s", ip);
    }
}

// Scan files from SD card
void FileViewerUI::scanFiles()
{
    m_files.clear();

    DIR *dir = opendir(UPLOAD_DIR);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to open directory: %s", UPLOAD_DIR);
        return;
    }

    struct dirent *entry;
    struct stat st;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, entry->d_name);

            if (stat(filepath, &st) == 0)
            {
                FileItem item;
                item.name = entry->d_name;
                item.size = st.st_size;
                item.selected = false;
                m_files.push_back(item);
            }
        }
    }

    closedir(dir);

    // Sort by name
    std::sort(m_files.begin(), m_files.end(),
              [](const FileItem &a, const FileItem &b)
              {
                  return a.name < b.name;
              });

    ESP_LOGI(TAG, "Found %d files", m_files.size());
}

// Update file list UI
void FileViewerUI::updateFileListUI()
{
    if (!m_fileList)
        return;

    // Clear list
    lv_obj_clean(m_fileList);

    if (m_files.empty())
    {
        lv_obj_t *item = lv_list_add_button(m_fileList, LV_SYMBOL_WARNING, "No files found");
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
        return;
    }

    // Add files to list
    for (size_t i = 0; i < m_files.size(); i++)
    {
        const FileItem &file = m_files[i];

        char label[256];
        snprintf(label, sizeof(label), "%s (%.1f KB)",
                 file.name.c_str(), file.size / 1024.0f);

        lv_obj_t *btn = lv_list_add_button(m_fileList, LV_SYMBOL_FILE, label);
        lv_obj_add_event_cb(btn, fileItemClicked, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(btn, (void *)i); // Store index

        // Highlight selected
        if ((int)i == m_selectedIndex)
        {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xc8e6c9), 0);
        }
    }
}

// Refresh file list
void FileViewerUI::refreshFileList()
{
    ESP_LOGI(TAG, "Refreshing file list...");

    if (m_labelStatus)
    {
        lv_label_set_text(m_labelStatus, "Status: Scanning files...");
    }

    scanFiles();
    updateFileListUI();

    if (m_labelStatus)
    {
        lv_label_set_text_fmt(m_labelStatus, "Status: %d file(s) found", m_files.size());
    }

    ESP_LOGI(TAG, "File list refreshed");
}

// Get selected file path
std::string FileViewerUI::getSelectedFilePath()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_files.size())
    {
        return "";
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s",
             UPLOAD_DIR, m_files[m_selectedIndex].name.c_str());

    return filepath;
}

// Render selected file
void FileViewerUI::renderSelected()
{
    if (m_selectedIndex < 0)
    {
        ESP_LOGW(TAG, "No file selected");
        if (m_labelStatus)
        {
            lv_label_set_text(m_labelStatus, "Status: No file selected");
        }
        return;
    }

    std::string filepath = getSelectedFilePath();
    ESP_LOGI(TAG, "Rendering: %s", filepath.c_str());

    if (m_labelStatus)
    {
        lv_label_set_text_fmt(m_labelStatus, "Rendering: %s", m_files[m_selectedIndex].name.c_str());
    }

    // Clear previous canvas
    clearCanvas();

    // Create canvas if needed
    if (!m_canvas)
    {
        // Canvas = full render screen size (landscape 1280×800)
        const uint16_t CANVAS_WIDTH = 1280;
        const uint16_t CANVAS_HEIGHT = 800;

        // Allocate buffer for canvas (RGB565) — must use PSRAM (2MB for 1280×800)
        size_t bufferSize = CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint16_t);
        m_canvasBuffer = heap_caps_malloc(bufferSize, MALLOC_CAP_SPIRAM);

        if (!m_canvasBuffer)
        {
            ESP_LOGE(TAG, "Failed to allocate canvas buffer");
            if (m_labelStatus)
            {
                lv_label_set_text(m_labelStatus, "Error: Out of memory");
            }
            return;
        }

        // Create canvas on render screen (full-screen, behind back button)
        m_canvas = new LVCanvas(
            m_renderScreen,
            CANVAS_WIDTH,
            CANVAS_HEIGHT,
            LV_COLOR_FORMAT_RGB565,
            m_canvasBuffer);

        // Center canvas in render screen
        lv_obj_align(m_canvas->obj(), LV_ALIGN_CENTER, 0, 0);
        lv_obj_move_background(m_canvas->obj()); // Behind back button

        // Fill with white background
        m_canvas->fill(LVColor::White);
    }

    // Create renderer if needed
    if (!m_renderer)
    {
        m_renderer = std::make_unique<JsonRenderer::JsonRenderer>(m_canvas);
    }

    // Render JSON
    if (m_renderer->loadAndRender(filepath.c_str()))
    {
        ESP_LOGI(TAG, "Render successful");
        if (m_labelStatus)
        {
            lv_label_set_text(m_labelStatus, "Status: Render complete ✓");
        }
        switchToRenderMode();
    }
    else
    {
        ESP_LOGE(TAG, "Render failed: %s", m_renderer->getLastError());
        if (m_labelStatus)
        {
            lv_label_set_text_fmt(m_labelStatus, "Error: %s", m_renderer->getLastError());
        }
    }
}

// Render file by name (called from web interface)
bool FileViewerUI::renderFile(const char *filename)
{
    if (!filename || strlen(filename) == 0)
    {
        ESP_LOGW(TAG, "Empty filename");
        return false;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);

    ESP_LOGI(TAG, "Rendering file: %s", filepath);

    // CRITICAL: Acquire LVGL lock for thread-safe UI operations
    lv_lock();

    // Create UI if not initialized
    if (!m_parent)
    {
        ESP_LOGI(TAG, "Creating FileViewerUI from LVGL task context...");
        create(lv_screen_active());
        if (!m_parent)
        {
            ESP_LOGE(TAG, "Failed to create FileViewerUI");
            lv_unlock();
            return false;
        }
    }

    // Clear previous canvas
    clearCanvas();

    // Create canvas if needed
    if (!m_canvas)
    {
        // Canvas = full render screen (landscape 1280×800)
        const uint16_t CANVAS_WIDTH = 1280;
        const uint16_t CANVAS_HEIGHT = 800;

        // Must use PSRAM — 1280×800×2 = 2MB exceeds internal RAM
        size_t bufferSize = CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint16_t);
        m_canvasBuffer = heap_caps_malloc(bufferSize, MALLOC_CAP_SPIRAM);

        if (!m_canvasBuffer)
        {
            ESP_LOGE(TAG, "Failed to allocate canvas buffer (%d bytes) in PSRAM", bufferSize);
            lv_unlock();
            return false;
        }

        // Create canvas directly on render screen (behind back button)
        m_canvas = new LVCanvas(
            m_renderScreen,
            CANVAS_WIDTH,
            CANVAS_HEIGHT,
            LV_COLOR_FORMAT_RGB565,
            m_canvasBuffer);

        lv_obj_align(m_canvas->obj(), LV_ALIGN_CENTER, 0, 0);
        lv_obj_move_background(m_canvas->obj()); // Behind back button
        m_canvas->fill(LVColor::White);
    }

    // Create renderer if needed
    if (!m_renderer)
    {
        m_renderer = std::make_unique<JsonRenderer::JsonRenderer>(m_canvas);
    }

    // Render JSON
    bool success = false;
    if (m_renderer->loadAndRender(filepath))
    {
        ESP_LOGI(TAG, "Render successful: %s", filename);
        if (m_labelStatus)
        {
            lv_label_set_text_fmt(m_labelStatus, "Rendered: %s ✓", filename);
        }
        // Switch to full-screen render mode
        switchToRenderMode();
        success = true;
    }
    else
    {
        ESP_LOGE(TAG, "Render failed: %s - %s", filename, m_renderer->getLastError());
        if (m_labelStatus)
        {
            lv_label_set_text_fmt(m_labelStatus, "Error: %s", m_renderer->getLastError());
        }
        success = false;
    }

    // CRITICAL: Release LVGL lock
    lv_unlock();

    return success;
}

// Switch to full-screen render mode (load render screen)
void FileViewerUI::switchToRenderMode()
{
    if (m_renderScreen)
        lv_screen_load(m_renderScreen);
    ESP_LOGI(TAG, "Switched to render mode");
}

// Switch back to file manager screen
void FileViewerUI::switchToFileMode()
{
    if (m_fileManagerContainer)
        lv_screen_load(m_fileManagerContainer);
    ESP_LOGI(TAG, "Switched to file manager mode");
}

std::string FileViewerUI::getLastDebugInfo() const
{
    if (m_renderer)
    {
        return m_renderer->getDebugInfo();
    }
    return "{}";
}

void FileViewerUI::setDebugMode(bool enable)
{
    if (m_renderer)
    {
        m_renderer->setDebugMode(enable);
    }
}

// Clear canvas
void FileViewerUI::clearCanvas()
{
    if (m_renderer)
    {
        m_renderer->clear();
        m_renderer.reset();
    }

    // Destroy canvas so it is recreated at correct size on next render
    if (m_canvas)
    {
        delete m_canvas;
        m_canvas = nullptr;
    }
    if (m_canvasBuffer)
    {
        heap_caps_free(m_canvasBuffer);
        m_canvasBuffer = nullptr;
    }

    ESP_LOGI(TAG, "Canvas cleared");
}

// ===== Event Handlers =====

void FileViewerUI::btnRefreshClicked(lv_event_t *e)
{
    FileViewerUI *ui = static_cast<FileViewerUI *>(lv_event_get_user_data(e));
    ui->refreshFileList();
}

void FileViewerUI::btnRenderClicked(lv_event_t *e)
{
    FileViewerUI *ui = static_cast<FileViewerUI *>(lv_event_get_user_data(e));
    ui->renderSelected();
}

void FileViewerUI::btnClearClicked(lv_event_t *e)
{
    FileViewerUI *ui = static_cast<FileViewerUI *>(lv_event_get_user_data(e));
    ui->clearCanvas();

    if (ui->m_labelStatus)
    {
        lv_label_set_text(ui->m_labelStatus, "Status: Canvas cleared");
    }
}

void FileViewerUI::btnBackClicked(lv_event_t *e)
{
    FileViewerUI *ui = static_cast<FileViewerUI *>(lv_event_get_user_data(e));
    ui->switchToFileMode();
}

void FileViewerUI::fileItemClicked(lv_event_t *e)
{
    FileViewerUI *ui = static_cast<FileViewerUI *>(lv_event_get_user_data(e));
    lv_obj_t *btn = static_cast<lv_obj_t *>(lv_event_get_target(e));

    if (!ui || !btn)
        return;

    size_t index = (size_t)lv_obj_get_user_data(btn);

    // Update selection
    ui->m_selectedIndex = index;
    ui->updateFileListUI(); // Refresh to show selection

    ESP_LOGI(TAG, "File selected: %s", ui->m_files[index].name.c_str());

    if (ui->m_labelStatus)
    {
        lv_label_set_text_fmt(ui->m_labelStatus, "Selected: %s", ui->m_files[index].name.c_str());
    }
}
