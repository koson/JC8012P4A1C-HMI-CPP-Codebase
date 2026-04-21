/**
 * @file simple_note_demo.cpp
 * @brief Simple note application demonstrating Document/View pattern
 * 
 * Features:
 * - LVDocument for data model with serialization
 * - Simple text editing with save/load to SD card
 * - Basic Document/View pattern without Navigator
 */

#include "LVDocument.hpp"
#include "LVScreen.hpp"
#include "LVTextArea.hpp"
#include "LVButton.hpp"
#include "LVLabel.hpp"
#include "LVStorage.hpp"
#include "main_menu.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "SimpleNoteDemo";
static const char* NOTE_FILE = "/sdcard/simple_note.txt";

// ============================================================================
// SimpleDocument - Document class for storing note text
// ============================================================================
class SimpleDocument : public LVDocument {
private:
    std::string m_text;

public:
    SimpleDocument() : LVDocument("SimpleDocument"), m_text("") {
        ESP_LOGI(TAG, "SimpleDocument created");
    }

    // Override serialize for save/load
    bool serialize(bool isLoading, std::string& data) override {
        if (isLoading) {
            // Load: copy data to m_text
            m_text = data;
            ESP_LOGI(TAG, "Loaded %d bytes", data.length());
            return true;
        } else {
            // Save: copy m_text to data
            data = m_text;
            ESP_LOGI(TAG, "Saved %d bytes", data.length());
            return true;
        }
    }

    // Getters/Setters
    const char* getText() const { return m_text.c_str(); }
    void setText(const char* text) { 
        m_text = text ? text : "";
        setModified(true);
    }
    size_t getLength() const { return m_text.length(); }
};

// ============================================================================
// SimpleNoteView - View class with UI
// ============================================================================
class SimpleNoteView : public LVScreen {
private:
    SimpleDocument* m_document;
    lv_obj_t* m_textAreaObj;
    lv_obj_t* m_statusLabelObj;

public:
    SimpleNoteView() : LVScreen(nullptr), m_document(nullptr), m_textAreaObj(nullptr), m_statusLabelObj(nullptr) {
        ESP_LOGI(TAG, "SimpleNoteView created");
        
        // Set screen background color
        lv_obj_set_style_bg_color(getObj(), lv_color_hex(0x2196F3), 0);
        lv_obj_set_style_bg_opa(getObj(), LV_OPA_COVER, 0);
    }

    ~SimpleNoteView() {
        if (m_document) {
            delete m_document;
        }
    }

    void setDocument(SimpleDocument* doc) {
        if (m_document && m_document != doc) {
            delete m_document;
        }
        m_document = doc;
        
        // Create UI if not already created
        if (!m_textAreaObj) {
            createUI();
        }
        
        updateUI();
    }

private:
    void createUI() {
        ESP_LOGI(TAG, "createUI() called, screen obj = %p", getObj());
        
        if (!getObj()) {
            ESP_LOGE(TAG, "ERROR: getObj() is NULL!");
            return;
        }
        
        // Title - create directly on screen object
        lv_obj_t* titleObj = lv_label_create(getObj());
        lv_label_set_text(titleObj, "Simple Note Demo");
        lv_obj_align(titleObj, LV_ALIGN_TOP_MID, 0, 10);
        lv_obj_set_style_text_color(titleObj, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(titleObj, &lv_font_montserrat_24, 0);
        
        ESP_LOGI(TAG, "Title created: obj=%p, parent=%p, visible=%d", 
                 titleObj, lv_obj_get_parent(titleObj),
                 lv_obj_is_visible(titleObj));

        
        // TextArea - create directly on screen
        lv_obj_t* textAreaObj = lv_textarea_create(getObj());
        lv_obj_set_size(textAreaObj, 900, 400);
        lv_obj_align(textAreaObj, LV_ALIGN_TOP_MID, 0, 50);
        lv_textarea_set_placeholder_text(textAreaObj, "Type your note here...");
        m_textAreaObj = textAreaObj;  // Store for later use
        
        ESP_LOGI(TAG, "TextArea created: obj=%p, parent=%p, visible=%d, w=%d, h=%d",
                 textAreaObj,
                 lv_obj_get_parent(textAreaObj),
                 lv_obj_is_visible(textAreaObj),
                 lv_obj_get_width(textAreaObj),
                 lv_obj_get_height(textAreaObj));

        // Status label - create directly
        lv_obj_t* statusObj = lv_label_create(getObj());
        lv_label_set_text(statusObj, "Ready");
        lv_obj_align(statusObj, LV_ALIGN_BOTTOM_MID, 0, -120);
        m_statusLabelObj = statusObj;
        ESP_LOGI(TAG, "Status label created");

        // Buttons - create directly with callbacks
        int btnY = -60;
        
        // New button
        lv_obj_t* btnNew = lv_btn_create(getObj());
        lv_obj_set_size(btnNew, 200, 50);
        lv_obj_align(btnNew, LV_ALIGN_BOTTOM_LEFT, 50, btnY);
        lv_obj_t* labelNew = lv_label_create(btnNew);
        lv_label_set_text(labelNew, "New");
        lv_obj_center(labelNew);
        lv_obj_set_user_data(btnNew, this);
        lv_obj_add_event_cb(btnNew, [](lv_event_t* e) {
            SimpleNoteView* view = static_cast<SimpleNoteView*>(lv_event_get_user_data(e));
            if (view) view->onNew();
        }, LV_EVENT_CLICKED, nullptr);

        // Save button
        lv_obj_t* btnSave = lv_btn_create(getObj());
        lv_obj_set_size(btnSave, 200, 50);
        lv_obj_align(btnSave, LV_ALIGN_BOTTOM_LEFT, 280, btnY);
        lv_obj_t* labelSave = lv_label_create(btnSave);
        lv_label_set_text(labelSave, "Save");
        lv_obj_center(labelSave);
        lv_obj_set_user_data(btnSave, this);
        lv_obj_add_event_cb(btnSave, [](lv_event_t* e) {
            SimpleNoteView* view = static_cast<SimpleNoteView*>(lv_event_get_user_data(e));
            if (view) view->onSave();
        }, LV_EVENT_CLICKED, nullptr);

        // Load button
        lv_obj_t* btnLoad = lv_btn_create(getObj());
        lv_obj_set_size(btnLoad, 200, 50);
        lv_obj_align(btnLoad, LV_ALIGN_BOTTOM_LEFT, 510, btnY);
        lv_obj_t* labelLoad = lv_label_create(btnLoad);
        lv_label_set_text(labelLoad, "Load");
        lv_obj_center(labelLoad);
        lv_obj_set_user_data(btnLoad, this);
        lv_obj_add_event_cb(btnLoad, [](lv_event_t* e) {
            SimpleNoteView* view = static_cast<SimpleNoteView*>(lv_event_get_user_data(e));
            if (view) view->onLoad();
        }, LV_EVENT_CLICKED, nullptr);

        // Back button
        lv_obj_t* btnBack = lv_btn_create(getObj());
        lv_obj_set_size(btnBack, 200, 50);
        lv_obj_align(btnBack, LV_ALIGN_BOTTOM_RIGHT, -50, btnY);
        lv_obj_t* labelBack = lv_label_create(btnBack);
        lv_label_set_text(labelBack, "Back");
        lv_obj_center(labelBack);
        lv_obj_add_event_cb(btnBack, [](lv_event_t* e) {
            return_to_main_menu();
        }, LV_EVENT_CLICKED, nullptr);
        
        ESP_LOGI(TAG, "All UI created with pure LVGL");
    }

    void updateUI() {
        if (m_document && m_textAreaObj) {
            const char* text = m_document->getText();
            lv_textarea_set_text(m_textAreaObj, text ? text : "");
            updateStatus();
        }
    }

    void updateStatus() {
        if (!m_statusLabelObj || !m_document) return;

        char status[128];
        snprintf(status, sizeof(status), "%s | Length: %d bytes", 
                 m_document->isModified() ? "Modified" : "Saved",
                 m_document->getLength());
        lv_label_set_text(m_statusLabelObj, status);
    }

    void onNew() {
        ESP_LOGI(TAG, "New document");
        if (m_document) {
            m_document->newDocument();
            lv_textarea_set_text(m_textAreaObj, "");
            lv_label_set_text(m_statusLabelObj, "New document created");
        }
    }

    void onSave() {
        ESP_LOGI(TAG, "Save document");
        if (!m_document) return;

        // Get text from TextArea
        const char* text = lv_textarea_get_text(m_textAreaObj);
        m_document->setText(text ? text : "");

        // Save to SD card
        if (m_document->saveAs(NOTE_FILE)) {
            lv_label_set_text(m_statusLabelObj, "Saved successfully!");
            ESP_LOGI(TAG, "Document saved to %s", NOTE_FILE);
        } else {
            lv_label_set_text(m_statusLabelObj, "Save failed!");
            ESP_LOGE(TAG, "Failed to save document");
        }

        updateStatus();
    }

    void onLoad() {
        ESP_LOGI(TAG, "Load document");
        if (!m_document) return;

        // Check if storage is ready
        LVStorage& storage = LVStorage::getInstance();
        if (!storage.isReady()) {
            lv_label_set_text(m_statusLabelObj, "SD card not ready!");
            ESP_LOGE(TAG, "Storage not ready");
            return;
        }

        // Load from SD card
        if (m_document->open(NOTE_FILE)) {
            lv_textarea_set_text(m_textAreaObj, m_document->getText());
            lv_label_set_text(m_statusLabelObj, "Loaded successfully!");
            ESP_LOGI(TAG, "Document loaded from %s", NOTE_FILE);
        } else {
            lv_label_set_text(m_statusLabelObj, "Load failed! (File not found?)");
            ESP_LOGW(TAG, "Failed to load document");
        }

        updateStatus();
    }
    
    void onBack() {
        return_to_main_menu();
    }
};

// ============================================================================
// Demo entry point
// ============================================================================
static SimpleNoteView* g_noteView = nullptr;

extern "C" void test_simple_note_demo() {
    ESP_LOGI(TAG, "=== Simple Note Demo ===");

    // Get storage instance
    LVStorage& storage = LVStorage::getInstance();
    
    // Check if already mounted
    if (!storage.isReady()) {
        ESP_LOGI(TAG, "SD card not mounted, attempting to mount...");
        
        // IMPORTANT: Unmount first to reset SDMMC hardware state
        // This prevents ESP_ERR_TIMEOUT (0x107) errors
        storage.unmount();
        vTaskDelay(pdMS_TO_TICKS(200));  // Give hardware time to reset
        
        if (!storage.mount(LVStorage::SD_CARD, "/sdcard")) {
            ESP_LOGE(TAG, "Failed to mount SD card!");
            ESP_LOGE(TAG, "HINT: Please press RESET button and try again");
            return;
        }
        ESP_LOGI(TAG, "SD card mounted successfully");
    } else {
        ESP_LOGI(TAG, "SD card already mounted");
    }

    // Create document
    SimpleDocument* doc = new SimpleDocument();
    doc->newDocument();

    // Create view
    if (g_noteView) {
        delete g_noteView;
    }
    g_noteView = new SimpleNoteView();
    g_noteView->load();
    g_noteView->setDocument(doc);

    ESP_LOGI(TAG, "Simple Note Demo started");
}

extern "C" void cleanup_simple_note_demo() {
    if (g_noteView) {
        delete g_noteView;
        g_noteView = nullptr;
    }
}
