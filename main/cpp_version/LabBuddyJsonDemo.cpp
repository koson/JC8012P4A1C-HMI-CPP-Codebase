#include "LabBuddyJsonDemo.hpp"
#include "esp_log.h"
#include <dirent.h>
#include <sys/stat.h>

static const char *LOG_TAG = "LabBuddyJsonDemo";

LabBuddyJsonDemo::LabBuddyJsonDemo(LVWidget *parent)
    : LVWidget(parent), m_canvas(nullptr), m_titleLabel(nullptr), m_statusLabel(nullptr), m_renderer(nullptr), m_canvasBuffer(nullptr)
{
}

LabBuddyJsonDemo::~LabBuddyJsonDemo()
{
    if (m_renderer)
    {
        m_renderer.reset();
    }
    if (m_canvas)
    {
        delete m_canvas;
    }
    if (m_titleLabel)
    {
        delete m_titleLabel;
    }
    if (m_statusLabel)
    {
        delete m_statusLabel;
    }
    if (m_canvasBuffer)
    {
        free(m_canvasBuffer);
    }
}

void LabBuddyJsonDemo::init()
{
    // Set background color
    setBackgroundColor(LVColor(245, 245, 245).raw()); // Light gray
    setSize(1024, 600);

    // Create title label
    m_titleLabel = new LVLabel(this);
    m_titleLabel->setText("LabBuddy Circuit Viewer");
    m_titleLabel->setPos(20, 10);
    m_titleLabel->setTextColor(LVColor::Black.raw());

    // Create status label
    m_statusLabel = new LVLabel(this);
    m_statusLabel->setText("Ready to load circuit...");
    m_statusLabel->setPos(20, 35);
    m_statusLabel->setTextColor(LVColor(100, 100, 100).raw());

    // Create canvas
    createCanvas();

    // List available files on SD card (using 8.3 short filename)
    listSDCardFiles("/sdcard/WORKSH~1");

    ESP_LOGI(LOG_TAG, "LabBuddyJsonDemo initialized");
}

void LabBuddyJsonDemo::createCanvas()
{
    // Allocate canvas buffer (RGB565 format: 2 bytes per pixel)
    size_t bufferSize = CANVAS_WIDTH * CANVAS_HEIGHT * 2;
    m_canvasBuffer = (uint8_t *)malloc(bufferSize);

    if (!m_canvasBuffer)
    {
        ESP_LOGE(LOG_TAG, "Failed to allocate canvas buffer");
        return;
    }

    // Create canvas (parent is lv_obj_t*)
    m_canvas = new LVCanvas(
        obj(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        LV_COLOR_FORMAT_RGB565,
        m_canvasBuffer);

    // Position canvas below title and status
    lv_obj_align(m_canvas->obj(), LV_ALIGN_TOP_LEFT, 0, 70);

    // Clear canvas (white background)
    m_canvas->fill(LVColor::White);

    // Create JSON renderer
    m_renderer = std::make_unique<JsonRenderer::JsonRenderer>(m_canvas);

    ESP_LOGI(LOG_TAG, "Canvas created: %dx%d", CANVAS_WIDTH, CANVAS_HEIGHT);
}

void LabBuddyJsonDemo::listSDCardFiles(const char *path)
{
    ESP_LOGI(LOG_TAG, "===========================================");
    ESP_LOGI(LOG_TAG, "Listing files in: %s", path);
    ESP_LOGI(LOG_TAG, "===========================================");

    DIR *dir = opendir(path);
    if (!dir)
    {
        ESP_LOGE(LOG_TAG, "Failed to open directory: %s", path);
        return;
    }

    struct dirent *entry;
    int fileCount = 0;

    while ((entry = readdir(dir)) != nullptr)
    {
        fileCount++;

        // Build full path
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        // Check if it's a directory or file
        struct stat st;
        if (stat(fullPath, &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                ESP_LOGI(LOG_TAG, "  [DIR]  %s", entry->d_name);
            }
            else
            {
                ESP_LOGI(LOG_TAG, "  [FILE] %s (size: %ld bytes)", entry->d_name, st.st_size);
            }
        }
        else
        {
            ESP_LOGI(LOG_TAG, "  [????] %s (stat failed)", entry->d_name);
        }
    }

    closedir(dir);

    ESP_LOGI(LOG_TAG, "Total entries: %d", fileCount);
    ESP_LOGI(LOG_TAG, "===========================================");
}

bool LabBuddyJsonDemo::loadCircuit(const char *filePath)
{
    ESP_LOGI(LOG_TAG, "Loading circuit: %s", filePath);

    // Update status
    std::string status = "Loading: ";
    status += filePath;
    m_statusLabel->setText(status.c_str());

    // Load and render JSON
    if (!m_renderer)
    {
        ESP_LOGE(LOG_TAG, "Renderer not initialized");
        m_statusLabel->setText("Error: Renderer not initialized");
        return false;
    }

    if (!m_renderer->loadAndRender(filePath))
    {
        ESP_LOGE(LOG_TAG, "Failed to load circuit: %s", m_renderer->getLastError());

        std::string error = "Error: ";
        error += m_renderer->getLastError();
        m_statusLabel->setText(error.c_str());
        m_statusLabel->setTextColor(LVColor::Red.raw());

        return false;
    }

    // Success - update status
    const JsonRenderer::Screen *screen = m_renderer->getScreen();
    if (screen)
    {
        std::string successMsg = "Loaded: ";
        successMsg += screen->title;
        successMsg += " (";
        successMsg += std::to_string(screen->widgets.size());
        successMsg += " widgets)";

        m_statusLabel->setText(successMsg.c_str());
        m_statusLabel->setTextColor(LVColor(0, 150, 0).raw()); // Green
    }
    else
    {
        m_statusLabel->setText("Loaded successfully");
        m_statusLabel->setTextColor(LVColor(0, 150, 0).raw());
    }

    ESP_LOGI(LOG_TAG, "Circuit loaded successfully");
    return true;
}

// ========== C wrapper for MenuDemoApplication ==========
extern "C" void create_labhappy_json_demo()
{
    static LabBuddyJsonDemo *demo = nullptr;

    if (demo)
    {
        delete demo;
    }

    demo = new LabBuddyJsonDemo(nullptr);
    demo->init();

    // Load default circuit from SD card
    // NOTE: FAT filesystem using 8.3 short filenames (LFN not enabled)
    // Files found: HALF_A~1.JSO, SVG1_C~1.JSO, SHAPES~1.JSO
    demo->loadCircuit("/sdcard/WORKSH~1/HALF_A~1.JSO");
}
