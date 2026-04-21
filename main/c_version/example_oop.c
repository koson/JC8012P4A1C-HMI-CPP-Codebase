#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "include/lvgl_button.h"
#include "include/lvgl_label.h"
#include "include/lvgl_panel.h"

/**
 * @brief OOP Style LVGL Example - คล้าย MFC Application
 * 
 * ตัวอย่างนี้แสดงการใช้งาน OOP wrapper สำหรับ LVGL
 * คล้ายกับการสร้าง Dialog หรือ Form ใน MFC/WinForms
 */

static const char *TAG = "OOP_Example";

// Global widgets (คล้าง member variables ใน MFC dialog class)
static LVPanel* main_panel = NULL;
static LVButton* btn_increment = NULL;
static LVButton* btn_decrement = NULL;
static LVButton* btn_reset = NULL;
static LVLabel* lbl_title = NULL;
static LVLabel* lbl_counter = NULL;

// Application state
static int counter = 0;

// ===== Event Handlers (คล้าง message handlers ใน MFC) =====

static void on_increment_clicked(LVWidget* widget, lv_event_t* event)
{
    counter++;
    ESP_LOGI(TAG, "Increment clicked! Counter = %d", counter);
    
    // Update label (คล้าง UpdateData() ใน MFC)
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
}

static void on_decrement_clicked(LVWidget* widget, lv_event_t* event)
{
    counter--;
    ESP_LOGI(TAG, "Decrement clicked! Counter = %d", counter);
    
    // Update label
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
}

static void on_reset_clicked(LVWidget* widget, lv_event_t* event)
{
    counter = 0;
    ESP_LOGI(TAG, "Reset clicked! Counter = %d", counter);
    
    // Update label
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
}

// ===== Dialog Initialization (คล้าง OnInitDialog ใน MFC) =====

void test_oop_example(void)
{
    ESP_LOGI(TAG, "Creating OOP-style UI...");
    
    // 1. สร้าง Main Panel (คล้าง Dialog Window)
    main_panel = lvpanel_create(NULL);  // NULL = สร้างบน screen root
    lvwidget_set_size(LVPANEL_TO_WIDGET(main_panel), 600, 400);
    lvwidget_set_align(LVPANEL_TO_WIDGET(main_panel), LV_ALIGN_CENTER, 0, 0);
    lvwidget_set_bg_color(LVPANEL_TO_WIDGET(main_panel), lv_color_hex(0x2C3E50));
    lvpanel_set_flex_flow(main_panel, LV_FLEX_FLOW_COLUMN);
    lvpanel_set_gap(main_panel, 20);
    lvpanel_set_padding(main_panel, 30);
    
    // 2. สร้าง Title Label (คล้าง Static Text Control)
    lbl_title = lvlabel_create(LVPANEL_TO_WIDGET(main_panel));
    lvlabel_set_text(lbl_title, "OOP Counter Demo");
    lvlabel_set_font(lbl_title, &lv_font_montserrat_32);
    lvlabel_set_text_color(lbl_title, lv_color_hex(0xECF0F1));
    lvwidget_set_align(LVLABEL_TO_WIDGET(lbl_title), LV_ALIGN_TOP_MID, 0, 0);
    
    // 3. สร้าง Counter Label (แสดงค่า counter)
    lbl_counter = lvlabel_create(LVPANEL_TO_WIDGET(main_panel));
    lvlabel_set_text_fmt(lbl_counter, "Count: %d", counter);
    lvlabel_set_font(lbl_counter, &lv_font_montserrat_48);
    lvlabel_set_text_color(lbl_counter, lv_color_hex(0x3498DB));
    lvwidget_set_align(LVLABEL_TO_WIDGET(lbl_counter), LV_ALIGN_CENTER, 0, -20);
    
    // 4. สร้าง Button Panel (container สำหรับปุ่ม)
    LVPanel* button_panel = lvpanel_create(LVPANEL_TO_WIDGET(main_panel));
    lvwidget_set_size(LVPANEL_TO_WIDGET(button_panel), 500, 80);
    lvpanel_set_flex_flow(button_panel, LV_FLEX_FLOW_ROW);
    lvpanel_set_gap(button_panel, 15);
    lvwidget_set_bg_color(LVPANEL_TO_WIDGET(button_panel), lv_color_hex(0x34495E));
    lvwidget_set_align(LVPANEL_TO_WIDGET(button_panel), LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // 5. สร้าง Increment Button (คล้าง CButton control)
    btn_increment = lvbutton_create(LVPANEL_TO_WIDGET(button_panel));
    lvbutton_set_text(btn_increment, "+");
    lvbutton_set_font(btn_increment, &lv_font_montserrat_32);
    lvwidget_set_size(LVBUTTON_TO_WIDGET(btn_increment), 140, 60);
    lvwidget_set_bg_color(LVBUTTON_TO_WIDGET(btn_increment), lv_color_hex(0x27AE60));
    
    // Set event handler (คล้าง ON_BN_CLICKED macro ใน MFC)
    lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn_increment), on_increment_clicked);
    
    // 6. สร้าง Reset Button
    btn_reset = lvbutton_create(LVPANEL_TO_WIDGET(button_panel));
    lvbutton_set_text(btn_reset, "Reset");
    lvbutton_set_font(btn_reset, &lv_font_montserrat_20);
    lvwidget_set_size(LVBUTTON_TO_WIDGET(btn_reset), 140, 60);
    lvwidget_set_bg_color(LVBUTTON_TO_WIDGET(btn_reset), lv_color_hex(0xE67E22));
    lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn_reset), on_reset_clicked);
    
    // 7. สร้าง Decrement Button
    btn_decrement = lvbutton_create(LVPANEL_TO_WIDGET(button_panel));
    lvbutton_set_text(btn_decrement, "-");
    lvbutton_set_font(btn_decrement, &lv_font_montserrat_32);
    lvwidget_set_size(LVBUTTON_TO_WIDGET(btn_decrement), 140, 60);
    lvwidget_set_bg_color(LVBUTTON_TO_WIDGET(btn_decrement), lv_color_hex(0xE74C3C));
    lvwidget_set_on_clicked(LVBUTTON_TO_WIDGET(btn_decrement), on_decrement_clicked);
    
    ESP_LOGI(TAG, "OOP UI created successfully!");
    ESP_LOGI(TAG, "  - Main Panel: %p", main_panel);
    ESP_LOGI(TAG, "  - Title Label: %p", lbl_title);
    ESP_LOGI(TAG, "  - Counter Label: %p", lbl_counter);
    ESP_LOGI(TAG, "  - Buttons: Inc=%p, Dec=%p, Reset=%p", 
             btn_increment, btn_decrement, btn_reset);
}

// ===== Dialog Cleanup (คล้าง OnDestroy ใน MFC) =====

void cleanup_oop_example(void)
{
    ESP_LOGI(TAG, "Cleaning up OOP UI...");
    
    // ทำลาย widgets (เรียง order จากใน -> นอก)
    // ใน real app อาจไม่จำเป็นต้องทำเพราะการลบ parent จะลบ children ด้วย
    
    if (main_panel) {
        lvpanel_destroy(main_panel);
        main_panel = NULL;
    }
    
    // Note: ไม่ต้อง destroy children เพราะ parent จะ destroy ให้อัตโนมัติ
    // (เหมือนกับ WM_DESTROY จะถูกส่งไปยัง child windows)
    
    ESP_LOGI(TAG, "Cleanup complete!");
}

// ===== Alternative: MFC-Style Class Structure (ถ้าอยากทำเป็น struct แบบเต็มรูปแบบ) =====

/**
 * @brief Counter Dialog Class (คล้าง CDialog derived class)
 * 
 * ตัวอย่างการจัดโครงสร้างแบบ MFC Dialog Class
 */
typedef struct {
    // Dialog data (คล้าง Dialog Data Exchange - DDX)
    int counter;
    
    // Controls (คล้าง member variables)
    LVPanel* main_panel;
    LVButton* btn_increment;
    LVButton* btn_decrement;
    LVButton* btn_reset;
    LVLabel* lbl_title;
    LVLabel* lbl_counter;
} CounterDialog;

static CounterDialog g_dialog;  // Global dialog instance

// Dialog methods (คล้าง member functions)
static void counter_dialog_on_init(CounterDialog* dlg);
static void counter_dialog_update_data(CounterDialog* dlg, bool save_to_controls);
static void counter_dialog_on_increment(LVWidget* widget, lv_event_t* event);
static void counter_dialog_on_decrement(LVWidget* widget, lv_event_t* event);
static void counter_dialog_on_reset(LVWidget* widget, lv_event_t* event);

// Implementation
static void counter_dialog_on_init(CounterDialog* dlg)
{
    dlg->counter = 0;
    
    // Create controls...
    // (same as test_oop_example above)
}

static void counter_dialog_update_data(CounterDialog* dlg, bool save_to_controls)
{
    if (!save_to_controls) {
        // Read from controls to variables (DDX)
        // ในกรณีนี้ไม่มีอะไรให้อ่านเพราะเป็น display-only
    } else {
        // Write from variables to controls (DDX)
        if (dlg->lbl_counter) {
            lvlabel_set_text_fmt(dlg->lbl_counter, "Count: %d", dlg->counter);
        }
    }
}

static void counter_dialog_on_increment(LVWidget* widget, lv_event_t* event)
{
    g_dialog.counter++;
    counter_dialog_update_data(&g_dialog, true);  // UpdateData(FALSE) ใน MFC
}

static void counter_dialog_on_decrement(LVWidget* widget, lv_event_t* event)
{
    g_dialog.counter--;
    counter_dialog_update_data(&g_dialog, true);
}

static void counter_dialog_on_reset(LVWidget* widget, lv_event_t* event)
{
    g_dialog.counter = 0;
    counter_dialog_update_data(&g_dialog, true);
}

void test_oop_dialog_class(void)
{
    ESP_LOGI(TAG, "Creating MFC-style Dialog...");
    counter_dialog_on_init(&g_dialog);
}
