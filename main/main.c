#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include "lv_demos.h"

extern void test_brightness_control();
extern void test_switch_control();
extern void test_button_control();
extern void test_checkbox_control();
extern void test_dropdown_selection();
extern void test_roller_selection();
extern void create_meter_demo();
extern void create_table_demo();
extern void test_timer_demo();
extern void test_event_demo();

void app_main(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * 10, // minimal draw buffer
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,   // ESP32-P4 supports DMA from PSRAM
            .buff_spiram = true,
            .sw_rotate = false,
        }
    };
    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    bsp_display_lock(0);

    // lv_demo_music();
    // lv_demo_benchmark();
    // lv_demo_widgets();

    // test_dropdown_selection(); // ✅ Phase 2 Test #7
    // test_roller_selection();   // ✅ Phase 2 Test #12: LVRoller
    // create_meter_demo();       // ✅ Phase 2 Test #16: LVMeter
    // create_table_demo();       // ✅ Phase 2 Test #17: LVTable
    // test_timer_demo();         // ✅ Phase 3.1: LVTimer
    test_event_demo();            // ⚙️  Phase 3.2: LVEvent

    bsp_display_unlock();
}
