#include "MenuDemoApplication.h"
#include "esp_log.h"

static const char *TAG = "MenuDemoApp";

// ========== Demo Creation Functions ==========
namespace
{

    // Demo 0: Widget Gallery
    void create_widget_gallery(lv_obj_t *parent)
    {
        // Slider
        lv_obj_t *slider_label = lv_label_create(parent);
        lv_label_set_text(slider_label, "Slider:");
        lv_obj_align(slider_label, LV_ALIGN_TOP_LEFT, 20, 100);

        lv_obj_t *slider = lv_slider_create(parent);
        lv_obj_set_width(slider, 300);
        lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 120, 100);
        lv_slider_set_value(slider, 50, LV_ANIM_OFF);

        lv_obj_t *slider_value = lv_label_create(parent);
        lv_label_set_text(slider_value, "50%");
        lv_obj_align(slider_value, LV_ALIGN_TOP_LEFT, 450, 100);

        lv_obj_add_event_cb(slider, [](lv_event_t *e)
                            {
        lv_obj_t* slider = static_cast<lv_obj_t*>(lv_event_get_target(e));
        lv_obj_t* label = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        int32_t value = lv_slider_get_value(slider);
        lv_label_set_text_fmt(label, "%d%%", (int)value); }, LV_EVENT_VALUE_CHANGED, slider_value);

        // Switch
        lv_obj_t *switch_label = lv_label_create(parent);
        lv_label_set_text(switch_label, "Switch:");
        lv_obj_align(switch_label, LV_ALIGN_TOP_LEFT, 20, 160);

        lv_obj_t *sw = lv_switch_create(parent);
        lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 120, 155);

        lv_obj_t *switch_status = lv_label_create(parent);
        lv_label_set_text(switch_status, "OFF");
        lv_obj_align(switch_status, LV_ALIGN_TOP_LEFT, 240, 160);

        lv_obj_add_event_cb(sw, [](lv_event_t *e)
                            {
        lv_obj_t* sw = static_cast<lv_obj_t*>(lv_event_get_target(e));
        lv_obj_t* label = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        lv_label_set_text(label, lv_obj_has_state(sw, LV_STATE_CHECKED) ? "ON" : "OFF"); }, LV_EVENT_VALUE_CHANGED, switch_status);

        // Buttons
        lv_obj_t *btn1 = lv_btn_create(parent);
        lv_obj_set_size(btn1, 150, 60);
        lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 20, 230);
        lv_obj_t *btn1_label = lv_label_create(btn1);
        lv_label_set_text(btn1_label, LV_SYMBOL_OK " Confirm");
        lv_obj_center(btn1_label);

        lv_obj_t *btn2 = lv_btn_create(parent);
        lv_obj_set_size(btn2, 150, 60);
        lv_obj_align(btn2, LV_ALIGN_TOP_LEFT, 190, 230);
        lv_obj_t *btn2_label = lv_label_create(btn2);
        lv_label_set_text(btn2_label, LV_SYMBOL_CLOSE " Cancel");
        lv_obj_center(btn2_label);

        // Checkbox
        lv_obj_t *cb1 = lv_checkbox_create(parent);
        lv_checkbox_set_text(cb1, "Option 1");
        lv_obj_align(cb1, LV_ALIGN_TOP_LEFT, 20, 320);

        lv_obj_t *cb2 = lv_checkbox_create(parent);
        lv_checkbox_set_text(cb2, "Option 2");
        lv_obj_align(cb2, LV_ALIGN_TOP_LEFT, 20, 370);
        lv_obj_add_state(cb2, LV_STATE_CHECKED);
    }

    // Demo 1: Touch Test
    void create_touch_test(lv_obj_t *parent)
    {
        static lv_obj_t *touch_indicator = nullptr;
        static lv_obj_t *touch_coord_label = nullptr;

        lv_obj_t *info = lv_label_create(parent);
        lv_label_set_text(info, "Touch anywhere on the screen");
        lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);

        touch_coord_label = lv_label_create(parent);
        lv_label_set_text(touch_coord_label, "No touch detected");
        lv_obj_set_style_text_font(touch_coord_label, &lv_font_montserrat_24, 0);
        lv_obj_align(touch_coord_label, LV_ALIGN_CENTER, 0, 0);

        touch_indicator = lv_obj_create(parent);
        lv_obj_set_size(touch_indicator, 60, 60);
        lv_obj_set_style_radius(touch_indicator, 30, 0);
        lv_obj_set_style_bg_color(touch_indicator, lv_color_hex(0xFF0000), 0);
        lv_obj_set_style_border_width(touch_indicator, 4, 0);
        lv_obj_set_style_border_color(touch_indicator, lv_color_hex(0xFFFFFF), 0);
        lv_obj_add_flag(touch_indicator, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_event_cb(parent, [](lv_event_t *e)
                            {
        static lv_obj_t* indicator = nullptr;
        static lv_obj_t* coord_label = nullptr;
        
        if (!indicator) {
            lv_obj_t* parent = static_cast<lv_obj_t*>(lv_event_get_target(e));
            for (uint32_t i = 0; i < lv_obj_get_child_count(parent); i++) {
                lv_obj_t* child = lv_obj_get_child(parent, i);
                if (lv_obj_get_width(child) == 60 && lv_obj_get_height(child) == 60) {
                    indicator = child;
                } else if (lv_obj_check_type(child, &lv_label_class)) {
                    const char* text = lv_label_get_text(child);
                    if (text && strstr(text, "No touch")) {
                        coord_label = child;
                    }
                }
            }
        }
        
        lv_event_code_t code = lv_event_get_code(e);
        lv_indev_t* indev = lv_indev_active();
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        
        if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
            if (indicator) {
                lv_obj_clear_flag(indicator, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_pos(indicator, point.x - 30, point.y - 30);
            }
            if (coord_label) {
                lv_label_set_text_fmt(coord_label, "X: %d, Y: %d", point.x, point.y);
            }
        } else if (code == LV_EVENT_RELEASED) {
            if (indicator) {
                lv_obj_add_flag(indicator, LV_OBJ_FLAG_HIDDEN);
            }
        } }, LV_EVENT_ALL, nullptr);
    }

    // Demo 2: Display Test
    void create_display_test(lv_obj_t *parent)
    {
        lv_obj_t *info = lv_label_create(parent);
        lv_label_set_text(info, "Display Resolution & Color Bars");
        lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);

        lv_obj_t *res_label = lv_label_create(parent);
        lv_label_set_text_fmt(res_label, "Resolution: %d x %d", LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_text_font(res_label, &lv_font_montserrat_20, 0);
        lv_obj_align(res_label, LV_ALIGN_TOP_MID, 0, 120);

        const lv_color_t colors[] = {
            lv_color_hex(0xFF0000), lv_color_hex(0x00FF00), lv_color_hex(0x0000FF),
            lv_color_hex(0xFFFF00), lv_color_hex(0xFF00FF), lv_color_hex(0x00FFFF),
            lv_color_hex(0xFFFFFF)};

        int bar_height = 60;
        int start_y = 200;

        for (int i = 0; i < 7; i++)
        {
            lv_obj_t *bar = lv_obj_create(parent);
            lv_obj_set_size(bar, 600, bar_height);
            lv_obj_set_pos(bar, (LV_HOR_RES - 600) / 2, start_y + (i * (bar_height + 5)));
            lv_obj_set_style_bg_color(bar, colors[i], 0);
            lv_obj_set_style_border_width(bar, 0, 0);
            lv_obj_set_style_pad_all(bar, 0, 0);
        }
    }

    // Demo 3: Simple Animation
    void create_simple_animation(lv_obj_t *parent)
    {
        lv_obj_t *info = lv_label_create(parent);
        lv_label_set_text(info, "Bouncing Ball Animation");
        lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);

        lv_obj_t *anim_obj = lv_obj_create(parent);
        lv_obj_set_size(anim_obj, 80, 80);
        lv_obj_set_style_radius(anim_obj, 40, 0);
        lv_obj_set_style_bg_color(anim_obj, lv_color_hex(0xFF5722), 0);
        lv_obj_set_pos(anim_obj, 100, 200);

        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, anim_obj);
        lv_anim_set_values(&anim, 100, LV_HOR_RES - 180);
        lv_anim_set_duration(&anim, 2000);
        lv_anim_set_playback_duration(&anim, 2000);
        lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
        lv_anim_set_exec_cb(&anim, [](void *var, int32_t value)
                            { lv_obj_set_x(static_cast<lv_obj_t *>(var), value); });
        lv_anim_start(&anim);

        lv_anim_t anim_y;
        lv_anim_init(&anim_y);
        lv_anim_set_var(&anim_y, anim_obj);
        lv_anim_set_values(&anim_y, 200, LV_VER_RES - 280);
        lv_anim_set_duration(&anim_y, 1500);
        lv_anim_set_playback_duration(&anim_y, 1500);
        lv_anim_set_repeat_count(&anim_y, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_path_cb(&anim_y, lv_anim_path_bounce);
        lv_anim_set_exec_cb(&anim_y, [](void *var, int32_t value)
                            { lv_obj_set_y(static_cast<lv_obj_t *>(var), value); });
        lv_anim_start(&anim_y);
    }

    // Demo 4: Color Test
    void create_color_test(lv_obj_t *parent)
    {
        lv_obj_t *info = lv_label_create(parent);
        lv_label_set_text(info, "RGB Color Gradient");
        lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 80);

        int box_size = 100;
        int spacing = 20;
        int start_x = 100;
        int start_y = 150;

        for (int i = 0; i < 5; i++)
        {
            lv_obj_t *box = lv_obj_create(parent);
            lv_obj_set_size(box, box_size, box_size);
            lv_obj_set_pos(box, start_x + (i * (box_size + spacing)), start_y);
            lv_obj_set_style_bg_color(box, lv_color_hex(0xFF0000 | (i * 0x003333)), 0);
            lv_obj_set_style_border_width(box, 2, 0);
        }

        start_y = 280;
        for (int i = 0; i < 5; i++)
        {
            lv_obj_t *box = lv_obj_create(parent);
            lv_obj_set_size(box, box_size, box_size);
            lv_obj_set_pos(box, start_x + (i * (box_size + spacing)), start_y);
            lv_obj_set_style_bg_color(box, lv_color_hex(0x00FF00 | (i * 0x330033)), 0);
            lv_obj_set_style_border_width(box, 2, 0);
        }

        start_y = 410;
        for (int i = 0; i < 5; i++)
        {
            lv_obj_t *box = lv_obj_create(parent);
            lv_obj_set_size(box, box_size, box_size);
            lv_obj_set_pos(box, start_x + (i * (box_size + spacing)), start_y);
            lv_obj_set_style_bg_color(box, lv_color_hex(0x0000FF | (i * 0x333300)), 0);
            lv_obj_set_style_border_width(box, 2, 0);
        }
    }

    // Demo 5: ColorWheel (from examples)
    extern "C" void create_colorwheel_demo();

    void create_colorwheel_wrapper(lv_obj_t *parent)
    {
        // Call the demo directly
        create_colorwheel_demo();
    }

    // Demo 6: LabHappy SVG Path Renderer
    extern "C" void create_labhappy_svg_demo();
    extern "C" void create_labhappy_json_demo();

    void create_labhappy_svg_wrapper(lv_obj_t *parent)
    {
        // Call the SVG demo directly
        create_labhappy_svg_demo();
    }

    void create_labhappy_json_wrapper(lv_obj_t *parent)
    {
        // Call the JSON demo directly
        create_labhappy_json_demo();
    }
} // namespace

// ========== MenuDemoApplication Implementation ==========

MenuDemoApplication &MenuDemoApplication::getInstance()
{
    static MenuDemoApplication instance;
    return instance;
}

MenuDemoApplication::MenuDemoApplication()
    : m_sysMgr(nullptr), m_demoManager(nullptr), m_initialized(false), m_running(false)
{
}

esp_err_t MenuDemoApplication::init(SystemManager &sysMgr)
{
    if (m_initialized)
    {
        ESP_LOGW(TAG, "Application already initialized");
        return ESP_OK;
    }

    m_sysMgr = &sysMgr;

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  LVGL C++ Demo Gallery");
    ESP_LOGI(TAG, "  Browse and run widget demos");
    ESP_LOGI(TAG, "===========================================");

    // Create DemoManager
    m_demoManager = std::make_unique<DemoManager>();
    ESP_LOGI(TAG, "✓ DemoManager created");

    // Register all demos
    registerAllDemos();

    m_initialized = true;
    ESP_LOGI(TAG, "✓ Menu Demo Application initialized with %d demos", m_demoManager->getDemoCount());
    return ESP_OK;
}

esp_err_t MenuDemoApplication::start()
{
    if (!m_initialized)
    {
        ESP_LOGE(TAG, "Application not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (m_running)
    {
        ESP_LOGW(TAG, "Application already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting Menu Demo Application...");

    // Lock display to create UI
    if (!m_sysMgr->lockDisplay())
    {
        ESP_LOGE(TAG, "Failed to lock display");
        return ESP_FAIL;
    }

    // Show menu using DemoManager
    ESP_LOGI(TAG, "Showing demo gallery menu...");
    m_demoManager->showMenu();

    m_sysMgr->unlockDisplay();

    m_running = true;
    ESP_LOGI(TAG, "✓ Menu Demo Application started");
    ESP_LOGI(TAG, "Touch any demo to run it!");

    return ESP_OK;
}

void MenuDemoApplication::stop()
{
    if (!m_running)
    {
        return;
    }

    ESP_LOGI(TAG, "Stopping Menu Demo Application...");
    m_running = false;
    ESP_LOGI(TAG, "✓ Menu Demo Application stopped");
}

// ========== Private Methods ==========

void MenuDemoApplication::registerAllDemos()
{
    ESP_LOGI(TAG, "Registering demos...");

    // Register basic widget demos
    m_demoManager->registerDemo(
        "Widget Gallery",
        "Browse LVGL widgets (Slider, Switch, Button, Checkbox)",
        create_widget_gallery);

    m_demoManager->registerDemo(
        "Touch Test",
        "Test touch functionality with visual feedback",
        create_touch_test);

    m_demoManager->registerDemo(
        "Display Test",
        "Test display features with color bars",
        create_display_test);

    m_demoManager->registerDemo(
        "Simple Animation",
        "Basic animation demo with bouncing ball",
        create_simple_animation);

    m_demoManager->registerDemo(
        "Color Test",
        "Display RGB color gradients",
        create_color_test);

    m_demoManager->registerDemo(
        "ColorWheel Picker",
        "Interactive color picker with HSV controls",
        create_colorwheel_wrapper);

    m_demoManager->registerDemo(
        "SVG Path Renderer",
        "LabHappy SVG symbol rendering (XOR/AND gates)",
        create_labhappy_svg_wrapper);

    m_demoManager->registerDemo(
        "JSON Circuit Viewer",
        "Load and render circuit from JSON file (embeddedSymbols v1.1)",
        create_labhappy_json_wrapper);

    ESP_LOGI(TAG, "✓ Registered %d demos", m_demoManager->getDemoCount());
}
