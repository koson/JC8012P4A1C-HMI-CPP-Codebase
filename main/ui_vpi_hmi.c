/*
 * VPI HMI User Interface Implementation
 * Landscape UI: 1024x600 with Electrical (left) and Tank (right) panels
 */

#include "ui_vpi_hmi.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "UI_VPI_HMI";

// Declare pressure needle image (from img_pressure_needle.c)
LV_IMAGE_DECLARE(img_pressure_needle);

// Screen dimensions - Rotated 90°: 800x1280 → 1280x800 (landscape)
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 800

// UI Objects
static lv_obj_t *main_screen = NULL;
static lv_obj_t *electrical_panel = NULL;
static lv_obj_t *tank_panel = NULL;
// static lv_obj_t *tank_graphic = NULL;
static lv_obj_t *status_label = NULL;
static lv_obj_t *pressure_label = NULL;

// Pressure gauge (scale widget - LVGL v9)
static lv_obj_t *pressure_gauge = NULL;
// static lv_obj_t *pressure_needle = NULL;  // OLD: Line needle (not used - using image needle)
static lv_obj_t *needle_triangle = NULL;  // Triangle at needle tip (using image needle API)
static lv_obj_t *gauge_value_label = NULL;
static lv_obj_t *gauge_unit_label = NULL;

// Phase labels
static lv_obj_t *phase1_voltage_label = NULL;
static lv_obj_t *phase1_current_label = NULL;
static lv_obj_t *phase1_power_label = NULL;
static lv_obj_t *phase1_reactive_label = NULL;
static lv_obj_t *phase1_energy_label = NULL;

static lv_obj_t *phase2_voltage_label = NULL;
static lv_obj_t *phase2_current_label = NULL;
static lv_obj_t *phase2_power_label = NULL;
static lv_obj_t *phase2_reactive_label = NULL;
static lv_obj_t *phase2_energy_label = NULL;

static lv_obj_t *phase3_voltage_label = NULL;
static lv_obj_t *phase3_current_label = NULL;
static lv_obj_t *phase3_power_label = NULL;
static lv_obj_t *phase3_reactive_label = NULL;
static lv_obj_t *phase3_energy_label = NULL;


// Helper function to create phase column (landscape layout)
static void create_phase_column(lv_obj_t *parent, const char *phase_name, uint32_t color,
                                 int x_offset, lv_obj_t **voltage, lv_obj_t **current,
                                 lv_obj_t **power, lv_obj_t **reactive, lv_obj_t **energy)
{
    int row_y[] = {98, 178, 258, 338, 418};  // Uniform 10px gaps
    int block_height = 70;
    int block_width = 180;

    // Phase header
    lv_obj_t *header = lv_label_create(parent);
    lv_label_set_text(header, phase_name);
    lv_obj_set_style_text_color(header, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(header, &lv_font_montserrat_18, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(header, 8, 0);
    lv_obj_set_pos(header, x_offset, 50);  // Start headers at Y=50
    lv_obj_set_size(header, block_width, 38);
    lv_obj_set_style_text_align(header, LV_TEXT_ALIGN_CENTER, 0);

    // Voltage
    *voltage = lv_label_create(parent);
    lv_label_set_text(*voltage, "225.1V");
    lv_obj_set_style_text_color(*voltage, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(*voltage, &lv_font_montserrat_32 , 0);
    lv_obj_set_style_bg_color(*voltage, lv_color_hex(0x303030), 0);
    lv_obj_set_style_bg_opa(*voltage, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_top(*voltage, 20, 0);  // Center vertically: (70-30)/2 ≈ 20
    lv_obj_set_pos(*voltage, x_offset, row_y[0]);
    lv_obj_set_size(*voltage, block_width, block_height);
    lv_obj_set_style_text_align(*voltage, LV_TEXT_ALIGN_CENTER, 0);
    
    // Current
    *current = lv_label_create(parent);
    lv_label_set_text(*current, "1.25A");
    lv_obj_set_style_text_color(*current, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(*current, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(*current, lv_color_hex(0x303030), 0);
    lv_obj_set_style_bg_opa(*current, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_top(*current, 20, 0);
    lv_obj_set_pos(*current, x_offset, row_y[1]);
    lv_obj_set_size(*current, block_width, block_height);
    lv_obj_set_style_text_align(*current, LV_TEXT_ALIGN_CENTER, 0);
    
    // Power
    *power = lv_label_create(parent);
    lv_label_set_text(*power, "---W");
    lv_obj_set_style_text_color(*power, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(*power, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(*power, lv_color_hex(0x303030), 0);
    lv_obj_set_style_bg_opa(*power, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_top(*power, 20, 0);
    lv_obj_set_pos(*power, x_offset, row_y[2]);
    lv_obj_set_size(*power, block_width, block_height);
    lv_obj_set_style_text_align(*power, LV_TEXT_ALIGN_CENTER, 0);
    
    // Reactive
    *reactive = lv_label_create(parent);
    lv_label_set_text(*reactive, "---VAR");
    lv_obj_set_style_text_color(*reactive, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(*reactive, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(*reactive, lv_color_hex(0x303030), 0);
    lv_obj_set_style_bg_opa(*reactive, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_top(*reactive, 20, 0);
    lv_obj_set_pos(*reactive, x_offset, row_y[3]);
    lv_obj_set_size(*reactive, block_width, block_height);
    lv_obj_set_style_text_align(*reactive, LV_TEXT_ALIGN_CENTER, 0);
    
    // Energy
    *energy = lv_label_create(parent);
    lv_label_set_text(*energy, "---kWh");
    lv_obj_set_style_text_color(*energy, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(*energy, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(*energy, lv_color_hex(0x303030), 0);
    lv_obj_set_style_bg_opa(*energy, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_top(*energy, 20, 0);
    lv_obj_set_pos(*energy, x_offset, row_y[4]);
    lv_obj_set_size(*energy, block_width, block_height);
    lv_obj_set_style_text_align(*energy, LV_TEXT_ALIGN_CENTER, 0);
}

void ui_vpi_hmi_create(void)
{
    ESP_LOGI(TAG, "Creating VPI HMI UI (Landscape 1280x800 with PPA rotation)...");
    
    // Get active screen
    main_screen = lv_scr_act();
    
    // Set black background
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_COVER, 0);
    
    // ========== LEFT PANEL: Electrical Parameters ==========
    electrical_panel = lv_obj_create(main_screen);
    lv_obj_set_size(electrical_panel, 800, 750);
    lv_obj_set_pos(electrical_panel, 10, 10);
    lv_obj_set_style_bg_color(electrical_panel, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_bg_opa(electrical_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(electrical_panel, 2, 0);
    lv_obj_set_style_border_color(electrical_panel, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_pad_all(electrical_panel, 10, 0);
    lv_obj_clear_flag(electrical_panel, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title
    lv_obj_t *elec_title = lv_label_create(electrical_panel);
    lv_label_set_text(elec_title, "Electrical parameters");
    lv_obj_set_style_text_color(elec_title, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(elec_title, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(elec_title, 10, 5);
    
    // Row labels (left side) - Consistent vertical spacing
    const char *row_labels[] = {"Voltage", "Current", "Power", "Reactive", "Energy"};
    int row_y[] = {98, 178, 258, 338, 418};  // Uniform 10px gaps between 70px blocks
    for (int i = 0; i < 5; i++) {
        lv_obj_t *label = lv_label_create(electrical_panel);
        lv_label_set_text(label, row_labels[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_pos(label, 10, row_y[i] + 20);  // Centered vertically in 70px block
    }
    
    // Create 3 phase columns - Uniform 35px horizontal gaps
    create_phase_column(electrical_panel, "Phase 1", 0xFFFF00, 145,  // Yellow
                        &phase1_voltage_label, &phase1_current_label,
                        &phase1_power_label, &phase1_reactive_label, &phase1_energy_label);
    
    create_phase_column(electrical_panel, "Phase 2", 0x00FFFF, 360,  // Cyan
                        &phase2_voltage_label, &phase2_current_label,
                        &phase2_power_label, &phase2_reactive_label, &phase2_energy_label);
    
    create_phase_column(electrical_panel, "Phase 3", 0xFF00FF, 575,  // Magenta
                        &phase3_voltage_label, &phase3_current_label,
                        &phase3_power_label, &phase3_reactive_label, &phase3_energy_label);
    
    // ========== RIGHT PANEL: Tank Parameters ==========
    tank_panel = lv_obj_create(main_screen);
    lv_obj_set_size(tank_panel, 440, 750);
    lv_obj_set_pos(tank_panel, SCREEN_WIDTH - 450, 10);
    lv_obj_set_style_bg_color(tank_panel, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_bg_opa(tank_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tank_panel, 2, 0);
    lv_obj_set_style_border_color(tank_panel, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_pad_all(tank_panel, 10, 0);
    lv_obj_clear_flag(tank_panel, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title
    lv_obj_t *tank_title = lv_label_create(tank_panel);
    lv_label_set_text(tank_title, "Tank parameters");
    lv_obj_set_style_text_color(tank_title, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(tank_title, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(tank_title, 10, 5);
    
    // ========== PRESSURE GAUGE (Scale Widget - LVGL v9) ==========
    pressure_gauge = lv_scale_create(tank_panel);
    lv_obj_set_size(pressure_gauge, 340, 340);
    lv_obj_align(pressure_gauge, LV_ALIGN_CENTER, 0, -50);
    
    // Configure as round scale
    lv_scale_set_mode(pressure_gauge, LV_SCALE_MODE_ROUND_INNER);
    
    // Background style
    lv_obj_set_style_bg_opa(pressure_gauge, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(pressure_gauge, lv_color_hex(0x2a2a2a), 0);
    lv_obj_set_style_radius(pressure_gauge, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(pressure_gauge, true, 0);
    lv_obj_set_style_border_width(pressure_gauge, 4, 0);  // Thicker border (was 2)
    lv_obj_set_style_border_color(pressure_gauge, lv_color_hex(0x00FF00), 0);
    
    // Configure scale range and ticks (0-100%)
    lv_scale_set_label_show(pressure_gauge, true);
    lv_scale_set_total_tick_count(pressure_gauge, 21);  // 21 ticks for 0-100 (every 5%)
    lv_scale_set_major_tick_every(pressure_gauge, 2);   // Major tick every 10%
    
    // Tick styling
    lv_obj_set_style_length(pressure_gauge, 8, LV_PART_ITEMS);       // Minor tick length
    lv_obj_set_style_length(pressure_gauge, 15, LV_PART_INDICATOR);  // Major tick length
    lv_obj_set_style_line_width(pressure_gauge, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_width(pressure_gauge, 3, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(pressure_gauge, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);
    lv_obj_set_style_line_color(pressure_gauge, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
    
    // Label styling (0-100 numbers) - make them visible!
    lv_obj_set_style_text_color(pressure_gauge, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);  // White text
    lv_obj_set_style_text_font(pressure_gauge, &lv_font_montserrat_20, LV_PART_INDICATOR);   // Larger font
    
    // Set range and angle
    lv_scale_set_range(pressure_gauge, 0, 100);     // 0-100%
    lv_scale_set_angle_range(pressure_gauge, 270);  // 270° arc
    lv_scale_set_rotation(pressure_gauge, 135);     // Start at 135°
    
    
    // Create image needle (from Inkscape SVG → LVGL converter)
    // Image must point to the RIGHT initially (→) as LVGL rotates from 0° (right)
    needle_triangle = lv_image_create(pressure_gauge);
    lv_image_set_src(needle_triangle, &img_pressure_needle);
    
    // Scale needle to 150% (256 = 100%, 384 = 150%, 512 = 200%)
    lv_image_set_scale(needle_triangle, 384);
    
    // Set pivot point (rotation center) - adjust X to match the circle on needle
    // For a 100x9 image with needle pointing right:
    // - X: Try 50 (middle of 100px width, where the pivot circle should be)
    // - Y: 4 (vertical center of 9px height)
    lv_image_set_pivot(needle_triangle, 4, 4);
    
    // Position needle at gauge center (NO offset - LVGL handles pivot automatically)
    lv_obj_align(needle_triangle, LV_ALIGN_CENTER, 45, 0);  
    
    // Use image needle API for automatic rotation
    lv_scale_set_image_needle_value(pressure_gauge, needle_triangle, 0);

    
    // Add center label with value
    // gauge_value_label = lv_label_create(pressure_gauge);
    // lv_label_set_text(gauge_value_label, "0");
    // lv_obj_set_style_text_color(gauge_value_label, lv_color_hex(0xFFFFFF), 0);
    // lv_obj_set_style_text_font(gauge_value_label, &lv_font_montserrat_32, 0);
    // lv_obj_align(gauge_value_label, LV_ALIGN_CENTER, 0, 30);
    
    // // Add % label
    // gauge_unit_label = lv_label_create(pressure_gauge);
    // lv_label_set_text(gauge_unit_label, "%");
    // lv_obj_set_style_text_color(gauge_unit_label, lv_color_hex(0xAAAAAA), 0);
    // lv_obj_set_style_text_font(gauge_unit_label, &lv_font_montserrat_16, 0);
    // lv_obj_align(gauge_unit_label, LV_ALIGN_CENTER, 0, 60);
    
    // Pressure label (digital readout below gauge)
    pressure_label = lv_label_create(tank_panel);
    lv_label_set_text(pressure_label, "0%");
    lv_obj_set_style_text_color(pressure_label, lv_color_hex(0x00FF00), 0);  // Green
    lv_obj_set_style_text_font(pressure_label, &lv_font_montserrat_32, 0);  // Larger font
    lv_obj_set_style_bg_color(pressure_label, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(pressure_label, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(pressure_label, 5, 0);  // More padding
    lv_obj_set_width(pressure_label, 150);  // Fixed width to keep centered
    lv_obj_set_style_text_align(pressure_label, LV_TEXT_ALIGN_CENTER, 0);  // Center text inside label
    lv_obj_align_to(pressure_label, pressure_gauge, LV_ALIGN_BOTTOM_MID, 0, -30);  // Below gauge, centered

    // ========== STATUS BAR ==========
    status_label = lv_label_create(main_screen);
    lv_label_set_text(status_label, "Status: Waiting for data...");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    


    ESP_LOGI(TAG, "VPI HMI UI created successfully (Landscape 1280x800)");
}

void ui_vpi_hmi_update_electrical(uint8_t phase, float voltage, float current, 
                                   float power, float reactive, float energy)
{
    char buf[128];  // Larger buffer for status string
    lv_obj_t *v_label, *c_label, *p_label, *r_label, *e_label;
    
    // Select phase labels
    switch (phase) {
        case 1:
            v_label = phase1_voltage_label;
            c_label = phase1_current_label;
            p_label = phase1_power_label;
            r_label = phase1_reactive_label;
            e_label = phase1_energy_label;
            break;
        case 2:
            v_label = phase2_voltage_label;
            c_label = phase2_current_label;
            p_label = phase2_power_label;
            r_label = phase2_reactive_label;
            e_label = phase2_energy_label;
            break;
        case 3:
            v_label = phase3_voltage_label;
            c_label = phase3_current_label;
            p_label = phase3_power_label;
            r_label = phase3_reactive_label;
            e_label = phase3_energy_label;
            break;
        default:
            return;
    }
    
    // Lock LVGL for thread-safe UI update
    if (bsp_display_lock(100)) {
        // Update values
        snprintf(buf, sizeof(buf), "%.1fV", voltage);
        lv_label_set_text(v_label, buf);
        
        snprintf(buf, sizeof(buf), "%.2fA", current);
        lv_label_set_text(c_label, buf);
        
        snprintf(buf, sizeof(buf), "%.1fW", power);
        lv_label_set_text(p_label, buf);
        
        snprintf(buf, sizeof(buf), "%.1fVAR", reactive);
        lv_label_set_text(r_label, buf);
        
        snprintf(buf, sizeof(buf), "%.2fkWh", energy);
        lv_label_set_text(e_label, buf);
        
        // Simple status update (within existing lock)
        static bool power_online = false;
        if (!power_online) {
            lv_label_set_text(status_label, "Status: Power meter: Online");
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x00FF00), 0);
            power_online = true;
        }
        
        bsp_display_unlock();
    } else {
        ESP_LOGW(TAG, "[UI] Failed to lock LVGL for electrical update");
    }
}

void ui_vpi_hmi_update_pressure(float percentage)
{
    char buf[32];
    
    ESP_LOGI(TAG, "[UI] Updating pressure gauge: %.0f%%", percentage);
    
    // Clamp percentage to gauge range (0-100)
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    // Lock LVGL for thread-safe UI update
    if (bsp_display_lock(100)) {
        // Update scale needle value
        if (pressure_gauge != NULL) {
            ESP_LOGI(TAG, "[UI] Needle set to value: %d%%", (int32_t)percentage);
            
            // Update needle color based on percentage zones
            lv_color_t needle_color;
            if (percentage < 50) {
                // Green zone: Safe (0-50%)
                needle_color = lv_color_hex(0x00FF00);
            } else if (percentage < 75) {
                // Yellow zone: Warning (50-75%)
                needle_color = lv_color_hex(0xFFFF00);
            } else {
                // Red zone: Danger (75-100%)
                needle_color = lv_color_hex(0xFF0000);
            }
            
            // OLD: Line needle color update (commented out)
            // lv_obj_set_style_line_color(pressure_needle, needle_color, LV_PART_MAIN);
            
            // Update image needle (LVGL auto-rotates it)
            if (needle_triangle != NULL) {
                // Recolor the image needle based on zone
                lv_obj_set_style_image_recolor(needle_triangle, needle_color, 0);
                lv_obj_set_style_image_recolor_opa(needle_triangle, LV_OPA_COVER, 0);
                
                // Update needle position
                lv_scale_set_image_needle_value(pressure_gauge, needle_triangle, (int32_t)percentage);
            }
            
            // Update center value label
            if (gauge_value_label != NULL) {
                snprintf(buf, sizeof(buf), "%.0f", percentage);
                lv_label_set_text(gauge_value_label, buf);
            }
        }
        bsp_display_unlock();
    } else {
        ESP_LOGW(TAG, "[UI] Failed to lock LVGL for gauge update");
    }
    
    // Update digital readout with color coding
    snprintf(buf, sizeof(buf), "%.0f%%", percentage);
    lv_label_set_text(pressure_label, buf);
    
    // Color code based on percentage zones
    if (percentage < 50) {
        // Green zone: Safe
        lv_obj_set_style_text_color(pressure_label, lv_color_hex(0x00FF00), 0);
    } else if (percentage < 75) {
        // Yellow zone: Warning
        lv_obj_set_style_text_color(pressure_label, lv_color_hex(0xFFFF00), 0);
    } else {
        // Red zone: Danger
        lv_obj_set_style_text_color(pressure_label, lv_color_hex(0xFF0000), 0);
    }
    
    // Simple status update (outside any lock to avoid deadlock)
    static bool pressure_online = false;
    if (!pressure_online && bsp_display_lock(-1)) {
        lv_label_set_text(status_label, "Status: Pressure: Online");
        lv_obj_set_style_text_color(status_label, lv_color_hex(0x00FF00), 0);
        pressure_online = true;
        bsp_display_unlock();
    }
}

void ui_vpi_hmi_update_status(const char *status)
{
    char buf[256];  // Larger buffer for UART status messages
    snprintf(buf, sizeof(buf), "Status: %s", status);
    lv_label_set_text(status_label, buf);
}

void ui_vpi_hmi_update(void)
{
    // This function is called periodically to update UI
    // Actual data updates happen through callback functions
}
