/**
 * @file event_demo.cpp
 * @brief Comprehensive demonstration of LVEvent system
 * 
 * This demo showcases:
 * 1. Event propagation (bubbling)
 * 2. Stop propagation
 * 3. Custom events
 * 4. Event filtering
 * 5. Multi-widget communication
 */

#include "lvgl.h"
#include "LVEvent.hpp"
#include "LVLabel.hpp"
#include "LVButton.hpp"
#include "LVSlider.hpp"
#include "LVSwitch.hpp"
#include "LVPanel.hpp"
#include <string>

static lv_obj_t* status_label;
static int event_counter = 0;

/**
 * Demo 1: Event Propagation & Bubbling
 * 
 * Shows how events propagate from child to parent, and how to stop it.
 */
void demo_event_propagation() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // Create parent container (persist for app lifetime)
    static LVPanel* parent = new LVPanel(nullptr);
    parent->setSize(480, 150);
    parent->setPos(10, 10);
    lv_obj_set_style_bg_color(parent->obj(), lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_bg_opa(parent->obj(), LV_OPA_20, 0);
    lv_obj_set_style_border_width(parent->obj(), 2, 0);
    lv_obj_set_style_border_color(parent->obj(), lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_radius(parent->obj(), 8, 0);

    // Parent label
    static LVLabel* parent_label = new LVLabel(parent);
    parent_label->setText("Parent Container");
    parent_label->setPos(10, 10);

    // Child button (allows propagation)
    static LVButton* child1 = new LVButton(parent);
    child1->setSize(200, 50);
    child1->setPos(10, 40);
    child1->setText("Child 1 (Propagates)");

    // Child button (stops propagation)
    static LVButton* child2 = new LVButton(parent);
    child2->setSize(200, 50);
    child2->setPos(10, 95);
    child2->setText("Child 2 (Stops)");

    // Parent event handler
    LVEventManager::addEvent(parent->obj(), LVEvent::Code::Clicked, [](LVEvent& e) {

        event_counter++;
        std::string msg = LV_SYMBOL_UP " Parent clicked (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
        printf("Parent received click event\n");
    });

    // Child 1 - allows bubbling
    LVEventManager::addEvent(child1->obj(), LVEvent::Code::Clicked, [](LVEvent& e) {
        printf("Child 1 clicked - event will bubble to parent\n");
    });

    // Child 2 - stops bubbling
    LVEventManager::addEvent(child2->obj(), LVEvent::Code::Clicked, [](LVEvent& e) {
        event_counter++;
        std::string msg = LV_SYMBOL_STOP " Child 2 clicked - stopped (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
        printf("Child 2 clicked - stopping propagation\n");
        e.stopPropagation();  // Stop event from reaching parent
    });
}

/**
 * Demo 2: Custom Events
 * 
 * Shows how to create and send custom events between widgets.
 */
void demo_custom_events() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // Define custom event codes (use high numbers to avoid conflicts)
    const lv_event_code_t EVENT_TEMPERATURE_ALERT = static_cast<lv_event_code_t>(LV_EVENT_PREPROCESS + 100);
    const lv_event_code_t EVENT_SYSTEM_READY = static_cast<lv_event_code_t>(LV_EVENT_PREPROCESS + 101);

    static LVPanel* panel = new LVPanel(nullptr);
    panel->setSize(480, 180);
    panel->setPos(500, 10);
    lv_obj_set_style_bg_color(panel->obj(), lv_color_hex(0xFF9800), 0);
    lv_obj_set_style_bg_opa(panel->obj(), LV_OPA_20, 0);
    lv_obj_set_style_border_width(panel->obj(), 2, 0);
    lv_obj_set_style_border_color(panel->obj(), lv_color_hex(0xFF9800), 0);
    lv_obj_set_style_radius(panel->obj(), 8, 0);

    static LVLabel* title = new LVLabel(panel);
    title->setText(LV_SYMBOL_SETTINGS " Custom Events");
    title->setPos(10, 10);

    static LVLabel* temp_label = new LVLabel(panel);
    temp_label->setText("Temperature: 25°C");
    temp_label->setPos(10, 40);

    // Slider to simulate temperature
    static LVSlider* temp_slider = new LVSlider(panel);
    temp_slider->setSize(300, 10);
    temp_slider->setPos(10, 70);
    temp_slider->setRange(0, 100);
    temp_slider->setValue(25);

    // Send custom event button
    static LVButton* send_btn = new LVButton(panel);
    send_btn->setSize(200, 45);
    send_btn->setPos(10, 90);
    send_btn->setText(std::string(LV_SYMBOL_CALL) + " Send System Ready");

    // Store panel pointer for event sending
    lv_obj_t* panel_obj = panel->obj();

    // Listen for custom temperature alert
    LVEventManager::addEvent(panel_obj, static_cast<LVEvent::Code>(EVENT_TEMPERATURE_ALERT), [](LVEvent& e) {
        int32_t* temp = static_cast<int32_t*>(e.getParam());
        event_counter++;
        std::string msg = LV_SYMBOL_WARNING " ALERT: Temp " + std::to_string(*temp) + 
                         "°C (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
        printf("Temperature alert: %d°C\n", *temp);
    });

    // Listen for system ready
    LVEventManager::addEvent(panel_obj, static_cast<LVEvent::Code>(EVENT_SYSTEM_READY), [](LVEvent& e) {
        event_counter++;
        std::string msg = LV_SYMBOL_OK " System ready (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
        printf("System ready event received\n");
    });

    // Store pointers for callbacks
    lv_obj_t* temp_slider_obj = temp_slider->obj();
    lv_obj_t* temp_label_obj = temp_label->obj();

    // Temperature slider value change
    LVEventManager::addEvent(temp_slider_obj, LVEvent::Code::ValueChanged, 
        [temp_slider_obj, temp_label_obj, panel_obj, EVENT_TEMPERATURE_ALERT](LVEvent& e) {
        int32_t temp = lv_slider_get_value(temp_slider_obj);
        std::string text = "Temperature: " + std::to_string(temp) + "°C";
        lv_label_set_text(temp_label_obj, text.c_str());

        // Send custom event if temperature too high
        if (temp > 70) {
            lv_obj_send_event(panel_obj, EVENT_TEMPERATURE_ALERT, &temp);
        }
    });

    // Send custom event button
    LVEventManager::addEvent(send_btn->obj(), LVEvent::Code::Clicked, [panel_obj, EVENT_SYSTEM_READY](LVEvent& e) {
        lv_obj_send_event(panel_obj, EVENT_SYSTEM_READY, nullptr);
    });
}

/**
 * Demo 3: Event Filtering & Multi-Event Handling
 * 
 * Shows how one widget can listen to multiple event types and filter them.
 */
void demo_event_filtering() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    static LVPanel* panel = new LVPanel(nullptr);
    panel->setSize(480, 160);
    panel->setPos(10, 170);
    lv_obj_set_style_bg_color(panel->obj(), lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_bg_opa(panel->obj(), LV_OPA_20, 0);
    lv_obj_set_style_border_width(panel->obj(), 2, 0);
    lv_obj_set_style_border_color(panel->obj(), lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_radius(panel->obj(), 8, 0);

    static LVLabel* title = new LVLabel(panel);
    title->setText(LV_SYMBOL_LIST " Event Filtering");
    title->setPos(10, 10);

    static LVLabel* event_log = new LVLabel(panel);
    event_log->setText("Press, long press, or click...");
    event_log->setPos(10, 40);

    static LVButton* interactive_btn = new LVButton(panel);
    interactive_btn->setSize(250, 60);
    interactive_btn->setPos(10, 70);
    interactive_btn->setText(std::string(LV_SYMBOL_EYE_OPEN) + " Interactive Button");

    lv_obj_t* event_log_obj = event_log->obj();

    // Listen to ALL events (for demonstration)
    LVEventManager::addEvent(interactive_btn->obj(), LVEvent::Code::All, [event_log_obj](LVEvent& e) {
        std::string msg;
        
        // Filter and handle different events
        switch (e.getCode()) {
            case LVEvent::Code::Pressed:
                msg = LV_SYMBOL_DOWNLOAD " Pressed";
                break;
            
            case LVEvent::Code::Pressing:
                // Too frequent, skip
                return;
            
            case LVEvent::Code::Released:
                msg = LV_SYMBOL_UPLOAD " Released";
                break;
            
            case LVEvent::Code::ShortClicked:
                msg = LV_SYMBOL_OK " Short click";
                break;
            
            case LVEvent::Code::Clicked:
                msg = LV_SYMBOL_RIGHT " Clicked";
                break;
            
            case LVEvent::Code::LongPressed:
                msg = LV_SYMBOL_WARNING " Long pressed!";
                break;
            
            case LVEvent::Code::LongPressedRepeat:
                msg = LV_SYMBOL_LOOP " Long press repeat";
                break;
            
            default:
                // Ignore other events
                return;
        }
        
        event_counter++;
        msg += " (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(event_log_obj, msg.c_str());
        lv_label_set_text(status_label, msg.c_str());
    });
}

/**
 * Demo 4: Multi-Widget Communication
 * 
 * Shows how multiple widgets can communicate through events.
 */
void demo_multi_widget_communication() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    static LVPanel* panel = new LVPanel(nullptr);
    panel->setSize(480, 250);
    panel->setPos(500, 200);
    lv_obj_set_style_bg_color(panel->obj(), lv_color_hex(0x9C27B0), 0);
    lv_obj_set_style_bg_opa(panel->obj(), LV_OPA_20, 0);
    lv_obj_set_style_border_width(panel->obj(), 2, 0);
    lv_obj_set_style_border_color(panel->obj(), lv_color_hex(0x9C27B0), 0);
    lv_obj_set_style_radius(panel->obj(), 8, 0);

    static LVLabel* title = new LVLabel(panel);
    title->setText(LV_SYMBOL_SHUFFLE " Widget Communication");
    title->setPos(10, 10);

    // Create interconnected widgets
    static LVLabel* brightness_label = new LVLabel(panel);
    brightness_label->setText("Brightness: 50%");
    brightness_label->setPos(10, 40);

    static LVSlider* brightness_slider = new LVSlider(panel);
    brightness_slider->setSize(300, 10);
    brightness_slider->setPos(10, 70);
    brightness_slider->setRange(0, 100);
    brightness_slider->setValue(50);

    static LVSwitch* enable_switch = new LVSwitch(panel);
    enable_switch->setPos(320, 65);
    lv_obj_add_state(enable_switch->obj(), LV_STATE_CHECKED);

    static LVLabel* mode_label = new LVLabel(panel);
    mode_label->setText("Mode: Normal");
    mode_label->setPos(10, 100);

    static LVButton* mode_btn = new LVButton(panel);
    mode_btn->setSize(150, 40);
    mode_btn->setPos(10, 130);
    mode_btn->setText(std::string(LV_SYMBOL_SETTINGS) + " Toggle Mode");

    static LVButton* reset_btn = new LVButton(panel);
    reset_btn->setSize(150, 40);
    reset_btn->setPos(170, 130);
    reset_btn->setText(std::string(LV_SYMBOL_REFRESH) + " Reset All");

    static LVLabel* stats = new LVLabel(panel);
    stats->setText("Changes: 0");
    stats->setPos(10, 180);

    static int change_count = 0;
    static bool turbo_mode = false;

    // Store object pointers for callbacks
    lv_obj_t* brightness_slider_obj = brightness_slider->obj();
    lv_obj_t* brightness_label_obj = brightness_label->obj();
    lv_obj_t* enable_switch_obj = enable_switch->obj();
    lv_obj_t* mode_label_obj = mode_label->obj();
    lv_obj_t* stats_obj = stats->obj();

    // Brightness slider updates label and enables switch
    LVEventManager::addEvent(brightness_slider_obj, LVEvent::Code::ValueChanged,
        [brightness_slider_obj, brightness_label_obj, enable_switch_obj, stats_obj](LVEvent& e) {
        int32_t value = lv_slider_get_value(brightness_slider_obj);
        
        // Update label
        std::string text = "Brightness: " + std::to_string(value) + "%";
        lv_label_set_text(brightness_label_obj, text.c_str());
        
        // Auto-disable if too low
        if (value < 10) {
            lv_obj_remove_state(enable_switch_obj, LV_STATE_CHECKED);
        }
        
        change_count++;
        std::string stats_text = "Changes: " + std::to_string(change_count);
        lv_label_set_text(stats_obj, stats_text.c_str());
    });

    // Switch controls slider enabled state
    LVEventManager::addEvent(enable_switch_obj, LVEvent::Code::ValueChanged,
        [brightness_slider_obj, enable_switch_obj](LVEvent& e) {
        bool enabled = lv_obj_has_state(enable_switch_obj, LV_STATE_CHECKED);
        if (enabled) {
            lv_obj_remove_state(brightness_slider_obj, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(brightness_slider_obj, LV_STATE_DISABLED);
        }
        
        event_counter++;
        std::string msg = std::string(LV_SYMBOL_POWER) + (enabled ? " Enabled" : " Disabled") + 
                         " (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
    });

    // Mode button affects slider range
    LVEventManager::addEvent(mode_btn->obj(), LVEvent::Code::Clicked,
        [brightness_slider_obj, mode_label_obj](LVEvent& e) {
        turbo_mode = !turbo_mode;
        if (turbo_mode) {
            lv_slider_set_range(brightness_slider_obj, 0, 200);  // Turbo mode!
            lv_label_set_text(mode_label_obj, "Mode: " LV_SYMBOL_CHARGE " Turbo");
        } else {
            lv_slider_set_range(brightness_slider_obj, 0, 100);
            lv_label_set_text(mode_label_obj, "Mode: Normal");
        }
        
        event_counter++;
        std::string msg = std::string(LV_SYMBOL_SETTINGS) + " Mode: " + 
                         (turbo_mode ? "Turbo" : "Normal") + 
                         " (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
    });

    // Reset button resets all widgets
    LVEventManager::addEvent(reset_btn->obj(), LVEvent::Code::Clicked,
        [brightness_slider_obj, enable_switch_obj, mode_label_obj, brightness_label_obj, stats_obj](LVEvent& e) {
        lv_slider_set_value(brightness_slider_obj, 50, LV_ANIM_OFF);
        lv_slider_set_range(brightness_slider_obj, 0, 100);
        lv_obj_add_state(enable_switch_obj, LV_STATE_CHECKED);
        lv_obj_remove_state(brightness_slider_obj, LV_STATE_DISABLED);
        turbo_mode = false;
        change_count = 0;
        
        lv_label_set_text(brightness_label_obj, "Brightness: 50%");
        lv_label_set_text(mode_label_obj, "Mode: Normal");
        lv_label_set_text(stats_obj, "Changes: 0");
        
        event_counter++;
        std::string msg = LV_SYMBOL_REFRESH " Reset complete (event #" + std::to_string(event_counter) + ")";
        lv_label_set_text(status_label, msg.c_str());
    });
}

/**
 * @brief Main event demo entry point
 */
extern "C" void test_event_demo() {
    // Create status bar at bottom
    status_label = lv_label_create(lv_screen_active());
    lv_obj_set_size(status_label, 1000, 40);
    lv_obj_set_pos(status_label, 10, 550);
    lv_label_set_text(status_label, LV_SYMBOL_PLAY " Event Demo Ready - Try clicking widgets!");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(status_label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(status_label, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(status_label, 10, 0);
    lv_obj_set_style_radius(status_label, 4, 0);

    // Run all demos
    demo_event_propagation();
    demo_custom_events();
    demo_event_filtering();
    demo_multi_widget_communication();

    printf("\n=== Event Demo Started ===\n");
    printf("Features demonstrated:\n");
    printf("1. Event Propagation & Stop Bubbling\n");
    printf("2. Custom Events\n");
    printf("3. Event Filtering (multi-event handling)\n");
    printf("4. Multi-Widget Communication\n");
    printf("Status shown at bottom of screen\n\n");
}
