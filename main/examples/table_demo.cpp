/**
 * @file table_demo.cpp
 * @brief ทดสอบ LVTable - Data table with cells
 */

#include "../include/LVPanel.hpp"
#include "../include/LVTable.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "esp_log.h"

static const char* TAG = "TableDemo";

class TableDemoScreen : public LVPanel {
public:
    TableDemoScreen() : LVPanel(nullptr) {
        setSize(LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(obj(), lv_color_hex(0x1E1E1E), LV_PART_MAIN);
        createUI();
    }

private:
    LVTable* productTable = nullptr;
    LVTable* scheduleTable = nullptr;
    LVLabel* statusLabel = nullptr;
    
    void createUI() {
        // Title
        auto* title = new LVLabel(this);
        title->setText("Table Demo");
        title->setTextColor(lv_color_hex(0xECF0F1));
        title->setFont(&lv_font_montserrat_24);
        title->setAlign(LV_ALIGN_TOP_MID);
        title->setPos(0, 10);
        
        // === Product Table ===
        auto* productTitle = new LVLabel(this);
        productTitle->setText("Product Inventory");
        productTitle->setTextColor(lv_color_hex(0x3498DB));
        productTitle->setFont(&lv_font_montserrat_16);
        productTitle->setAlign(LV_ALIGN_TOP_LEFT);
        productTitle->setPos(20, 60);
        
        productTable = new LVTable(this);
        productTable->setSize(760, 250);
        productTable->setAlign(LV_ALIGN_TOP_LEFT);
        productTable->setPos(20, 90);
        
        // Configure table
        productTable->setRowCount(6);    // Header + 5 products
        productTable->setColumnCount(4); // ID, Name, Stock, Price
        
        // Set column widths
        productTable->setColumnWidth(0, 80);  // ID
        productTable->setColumnWidth(1, 300); // Name
        productTable->setColumnWidth(2, 150); // Stock
        productTable->setColumnWidth(3, 150); // Price
        
        // Style the table
        lv_obj_set_style_bg_color(productTable->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_border_width(productTable->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(productTable->obj(), lv_color_hex(0x3498DB), LV_PART_MAIN);
        lv_obj_set_style_text_color(productTable->obj(), lv_color_hex(0xECF0F1), LV_PART_ITEMS);
        
        // Header row (row 0) - with MERGE and CROP control
        productTable->setCellValue(0, 0, "ID");
        productTable->setCellValue(0, 1, "Product Name");
        productTable->setCellValue(0, 2, "Stock");
        productTable->setCellValue(0, 3, "Price (฿)");
        
        // Make header bold/centered (using cell control)
        for (uint32_t col = 0; col < 4; col++) {
            productTable->setCellControl(0, col, LV_TABLE_CELL_CTRL_MERGE_RIGHT);
            lv_obj_set_style_bg_color(productTable->obj(), lv_color_hex(0x34495E), LV_PART_ITEMS | LV_STATE_DEFAULT);
        }
        
        // Fill data rows
        const char* products[5][4] = {
            {"001", "ESP32-P4 Dev Board", "25", "1,200"},
            {"002", "LVGL Display 7\"", "12", "2,500"},
            {"003", "USB-C Cable 2m", "150", "120"},
            {"004", "Power Adapter 5V/3A", "80", "250"},
            {"005", "Breadboard 830pts", "45", "85"}
        };
        
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 4; col++) {
                productTable->setCellValue(row + 1, col, products[row][col]);
            }
        }
        
        // === Schedule Table ===
        auto* scheduleTitle = new LVLabel(this);
        scheduleTitle->setText("Weekly Schedule");
        scheduleTitle->setTextColor(lv_color_hex(0xE74C3C));
        scheduleTitle->setFont(&lv_font_montserrat_16);
        scheduleTitle->setAlign(LV_ALIGN_TOP_LEFT);
        scheduleTitle->setPos(20, 360);
        
        scheduleTable = new LVTable(this);
        scheduleTable->setSize(760, 200);
        scheduleTable->setAlign(LV_ALIGN_TOP_LEFT);
        scheduleTable->setPos(20, 390);
        
        // Configure schedule table
        scheduleTable->setRowCount(4);  // Header + 3 time slots
        scheduleTable->setColumnCount(6); // Time, Mon-Fri
        
        // Set column widths
        scheduleTable->setColumnWidth(0, 120);  // Time
        for (int i = 1; i <= 5; i++) {
            scheduleTable->setColumnWidth(i, 125);  // Days
        }
        
        // Style schedule table
        lv_obj_set_style_bg_color(scheduleTable->obj(), lv_color_hex(0x2C3E50), LV_PART_MAIN);
        lv_obj_set_style_border_width(scheduleTable->obj(), 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(scheduleTable->obj(), lv_color_hex(0xE74C3C), LV_PART_MAIN);
        lv_obj_set_style_text_color(scheduleTable->obj(), lv_color_hex(0xECF0F1), LV_PART_ITEMS);
        
        // Header
        scheduleTable->setCellValue(0, 0, "Time");
        scheduleTable->setCellValue(0, 1, "Mon");
        scheduleTable->setCellValue(0, 2, "Tue");
        scheduleTable->setCellValue(0, 3, "Wed");
        scheduleTable->setCellValue(0, 4, "Thu");
        scheduleTable->setCellValue(0, 5, "Fri");
        
        // Time slots
        const char* schedule[3][6] = {
            {"09:00-12:00", "Meeting", "Dev", "Dev", "Review", "Planning"},
            {"13:00-15:00", "Dev", "Dev", "Meeting", "Dev", "Demo"},
            {"15:00-17:00", "Testing", "Debug", "Testing", "Deploy", "Free"}
        };
        
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 6; col++) {
                scheduleTable->setCellValue(row + 1, col, schedule[row][col]);
            }
        }
        
        // Status Label
        statusLabel = new LVLabel(this);
        statusLabel->setText("Status: Table initialized with sample data");
        statusLabel->setTextColor(lv_color_hex(0x95A5A6));
        statusLabel->setFont(&lv_font_montserrat_14);
        statusLabel->setAlign(LV_ALIGN_BOTTOM_MID);
        statusLabel->setPos(0, -10);
        
        // Add click handlers for cell selection
        lv_obj_add_event_cb(productTable->obj(), 
            [](lv_event_t* e) {
                auto* table = (LVTable*)lv_event_get_user_data(e);
                uint32_t row, col;
                table->getSelectedCell(&row, &col);
                
                char buf[128];
                const char* value = table->getCellValue(row, col);
                snprintf(buf, sizeof(buf), "Product selected: Row %d, Col %d = '%s'", 
                         (int)row, (int)col, value ? value : "");
                ESP_LOGI("TableDemo", "%s", buf);
            }, 
            LV_EVENT_CLICKED, productTable);
        
        lv_obj_add_event_cb(scheduleTable->obj(), 
            [](lv_event_t* e) {
                auto* table = (LVTable*)lv_event_get_user_data(e);
                uint32_t row, col;
                table->getSelectedCell(&row, &col);
                
                char buf[128];
                const char* value = table->getCellValue(row, col);
                snprintf(buf, sizeof(buf), "Schedule selected: Row %d, Col %d = '%s'", 
                         (int)row, (int)col, value ? value : "");
                ESP_LOGI("TableDemo", "%s", buf);
            }, 
            LV_EVENT_CLICKED, scheduleTable);
        
        ESP_LOGI("TableDemo", "Table demo initialized - 2 tables created");
        ESP_LOGI("TableDemo", "Product table: %dx%d", productTable->getRowCount(), productTable->getColumnCount());
        ESP_LOGI("TableDemo", "Schedule table: %dx%d", scheduleTable->getRowCount(), scheduleTable->getColumnCount());
    }
};

// Entry point
extern "C" void create_table_demo()
{
    ESP_LOGI(TAG, "Creating table demo...");
    
    auto* screen = lv_screen_active();
    auto* panel = new TableDemoScreen();
    lv_obj_set_parent(panel->obj(), screen);
    
    ESP_LOGI(TAG, "Table demo loaded");
}
