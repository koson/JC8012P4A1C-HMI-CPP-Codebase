/**
 * @file ElectricalPanel.cpp
 * @brief 3-Phase Electrical Parameters Panel Implementation
 */

#include "ElectricalPanel.h"
#include "bsp/esp-bsp.h"
#include <cstdio>

ElectricalPanel::ElectricalPanel(lv_obj_t* parent)
    : panel_(nullptr)
    , title_(nullptr)
{
    // Create main panel container (below header bar)
    panel_ = lv_obj_create(parent);
    lv_obj_set_size(panel_, 800, 680);  // Height reduced to 680 to leave space for status bar
    lv_obj_set_pos(panel_, 10, 70);     // Y=70 to leave space for header (60px + 10px gap)
    lv_obj_set_style_bg_color(panel_, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_bg_opa(panel_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel_, 2, 0);
    lv_obj_set_style_border_color(panel_, lv_color_hex(0x00FF00), 0);  // Green border
    lv_obj_set_style_pad_all(panel_, 10, 0);
    lv_obj_clear_flag(panel_, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create UI elements
    createTitle();
    createRowLabels();
    
    // Create 3 phase columns - Uniform 35px horizontal gaps
    createPhaseColumn("Phase 1", 0xFFFF00, 145, phase1_);  // Yellow
    createPhaseColumn("Phase 2", 0x00FFFF, 360, phase2_);  // Cyan
    createPhaseColumn("Phase 3", 0xFF00FF, 575, phase3_);  // Magenta
}

ElectricalPanel::~ElectricalPanel()
{
    // LVGL will handle cleanup when parent is deleted
}

void ElectricalPanel::updatePhase(uint8_t phase, float voltage, float current, 
                                   float power, float reactive, float energy)
{
    PhaseLabels* labels = nullptr;
    
    // Select phase labels
    switch (phase) {
        case 1: labels = &phase1_; break;
        case 2: labels = &phase2_; break;
        case 3: labels = &phase3_; break;
        default: return;  // Invalid phase
    }
    
    // Update labels (with display lock)
    if (bsp_display_lock(100)) {
        updatePhaseLabels(*labels, voltage, current, power, reactive, energy);
        bsp_display_unlock();
    }
}

void ElectricalPanel::updateAll(float v1, float v2, float v3,
                                float a1, float a2, float a3,
                                float w1, float w2, float w3,
                                float var1, float var2, float var3,
                                float kwh1, float kwh2, float kwh3)
{
    // Update all phases at once (with single lock)
    if (bsp_display_lock(100)) {
        updatePhaseLabels(phase1_, v1, a1, w1, var1, kwh1);
        updatePhaseLabels(phase2_, v2, a2, w2, var2, kwh2);
        updatePhaseLabels(phase3_, v3, a3, w3, var3, kwh3);
        bsp_display_unlock();
    }
}

void ElectricalPanel::setPosition(int x, int y)
{
    lv_obj_set_pos(panel_, x, y);
}

void ElectricalPanel::setSize(int width, int height)
{
    lv_obj_set_size(panel_, width, height);
}

void ElectricalPanel::createTitle()
{
    title_ = lv_label_create(panel_);
    lv_label_set_text(title_, "Electrical parameters");
    lv_obj_set_style_text_color(title_, lv_color_hex(0x00FF00), 0);  // Green
    lv_obj_set_style_text_font(title_, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(title_, 10, 5);
}

void ElectricalPanel::createRowLabels()
{
    const char* row_labels[] = {"Voltage", "Current", "Power", "Reactive", "Energy"};
    int row_y[] = {98, 178, 258, 338, 418};  // Uniform 10px gaps between 70px blocks
    
    for (int i = 0; i < 5; i++) {
        lv_obj_t* label = lv_label_create(panel_);
        lv_label_set_text(label, row_labels[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);  // White
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_pos(label, 10, row_y[i] + 20);  // Centered vertically in 70px block
    }
}

void ElectricalPanel::createPhaseColumn(const char* phase_name, uint32_t color, 
                                       int x_offset, PhaseLabels& labels)
{
    int row_y[] = {98, 178, 258, 338, 418};  // Uniform 10px gaps
    int block_height = 70;
    int block_width = 180;

    // Phase header
    lv_obj_t* header = lv_label_create(panel_);
    lv_label_set_text(header, phase_name);
    lv_obj_set_style_text_color(header, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(header, &lv_font_montserrat_18, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(header, 8, 0);
    lv_obj_set_pos(header, x_offset, 50);  // Start headers at Y=50
    lv_obj_set_size(header, block_width, 38);
    lv_obj_set_style_text_align(header, LV_TEXT_ALIGN_CENTER, 0);

    // Create value labels (Voltage, Current, Power, Reactive, Energy)
    lv_obj_t** label_ptrs[] = {
        &labels.voltage, &labels.current, &labels.power, 
        &labels.reactive, &labels.energy
    };
    const lv_font_t* fonts[] = {
        &lv_font_montserrat_32,  // Voltage - largest
        &lv_font_montserrat_24,  // Current
        &lv_font_montserrat_24,  // Power
        &lv_font_montserrat_24,  // Reactive
        &lv_font_montserrat_24   // Energy
    };
    const char* defaults[] = {"---V", "---A", "---W", "---VAR", "---kWh"};
    
    for (int i = 0; i < 5; i++) {
        *label_ptrs[i] = lv_label_create(panel_);
        lv_label_set_text(*label_ptrs[i], defaults[i]);
        lv_obj_set_style_text_color(*label_ptrs[i], lv_color_hex(color), 0);
        lv_obj_set_style_text_font(*label_ptrs[i], fonts[i], 0);
        lv_obj_set_style_bg_color(*label_ptrs[i], lv_color_hex(0x303030), 0);
        lv_obj_set_style_bg_opa(*label_ptrs[i], LV_OPA_COVER, 0);
        lv_obj_set_style_pad_top(*label_ptrs[i], 20, 0);  // Center vertically
        lv_obj_set_pos(*label_ptrs[i], x_offset, row_y[i]);
        lv_obj_set_size(*label_ptrs[i], block_width, block_height);
        lv_obj_set_style_text_align(*label_ptrs[i], LV_TEXT_ALIGN_CENTER, 0);
    }
}

void ElectricalPanel::updatePhaseLabels(const PhaseLabels& labels, 
                                       float voltage, float current, 
                                       float power, float reactive, float energy)
{
    char buf[32];
    
    // Update voltage
    snprintf(buf, sizeof(buf), "%.1fV", voltage);
    lv_label_set_text(labels.voltage, buf);
    
    // Update current
    snprintf(buf, sizeof(buf), "%.2fA", current);
    lv_label_set_text(labels.current, buf);
    
    // Update power
    snprintf(buf, sizeof(buf), "%.1fW", power);
    lv_label_set_text(labels.power, buf);
    
    // Update reactive
    snprintf(buf, sizeof(buf), "%.1fVAR", reactive);
    lv_label_set_text(labels.reactive, buf);
    
    // Update energy
    snprintf(buf, sizeof(buf), "%.2fkWh", energy);
    lv_label_set_text(labels.energy, buf);
}
