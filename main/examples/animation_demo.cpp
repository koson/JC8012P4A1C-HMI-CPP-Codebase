#include "lvgl.h"
#include "LVAnimation.hpp"
#include "LVStyle.hpp"
#include "LVColor.hpp"

extern "C" void test_animation_demo()
{
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "LVAnimation Demo");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // === Demo 1: Position Animation (Bounce) ===
    lv_obj_t* box1 = lv_obj_create(scr);
    lv_obj_set_size(box1, 60, 60);
    lv_obj_set_pos(box1, 20, 60);
    lv_obj_set_style_bg_color(box1, lv_color_hex(0xe74c3c), 0);
    lv_obj_set_style_radius(box1, 8, 0);
    
    lv_obj_t* label1 = lv_label_create(scr);
    lv_label_set_text(label1, "Bounce X");
    lv_obj_set_style_text_color(label1, lv_color_white(), 0);
    lv_obj_align_to(label1, box1, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    // Create animation for box1
    LVAnimation* anim1 = new LVAnimation(box1);
    anim1->setProperty(LVAnimation::Property::X)
         .setValues(20, 300)
         .setDuration(1500)
         .setPath(LVAnimation::Path::Bounce)
         .setRepeatCount(-1)
         .setPlayback(true)
         .setPlaybackDelay(200);
    anim1->start();
    
    // === Demo 2: Size Animation (EaseInOut) ===
    lv_obj_t* box2 = lv_obj_create(scr);
    lv_obj_set_size(box2, 60, 60);
    lv_obj_set_pos(box2, 400, 60);
    lv_obj_set_style_bg_color(box2, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(box2, 30, 0);
    
    lv_obj_t* label2 = lv_label_create(scr);
    lv_label_set_text(label2, "Grow/Shrink");
    lv_obj_set_style_text_color(label2, lv_color_white(), 0);
    lv_obj_align_to(label2, box2, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    LVAnimation* anim2 = new LVAnimation(box2);
    anim2->setProperty(LVAnimation::Property::Width)
         .setValues(60, 120)
         .setDuration(1000)
         .setPath(LVAnimation::Path::EaseInOut)
         .setRepeatCount(-1)
         .setPlayback(true);
    anim2->start();
    
    // Height animation for same box
    LVAnimation* anim2b = new LVAnimation(box2);
    anim2b->setProperty(LVAnimation::Property::Height)
          .setValues(60, 120)
          .setDuration(1000)
          .setPath(LVAnimation::Path::EaseInOut)
          .setRepeatCount(-1)
          .setPlayback(true);
    anim2b->start();
    
    // === Demo 3: Opacity Animation (Linear) ===
    lv_obj_t* box3 = lv_obj_create(scr);
    lv_obj_set_size(box3, 100, 60);
    lv_obj_set_pos(box3, 720, 60);
    lv_obj_set_style_bg_color(box3, lv_color_hex(0x2ecc71), 0);
    lv_obj_set_style_radius(box3, 8, 0);
    
    lv_obj_t* label3 = lv_label_create(scr);
    lv_label_set_text(label3, "Fade In/Out");
    lv_obj_set_style_text_color(label3, lv_color_white(), 0);
    lv_obj_align_to(label3, box3, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    LVAnimation* anim3 = new LVAnimation(box3);
    anim3->setProperty(LVAnimation::Property::Opacity)
         .setValues(LV_OPA_TRANSP, LV_OPA_COVER)
         .setDuration(800)
         .setPath(LVAnimation::Path::Linear)
         .setRepeatCount(-1)
         .setPlayback(true);
    anim3->start();
    
    // === Demo 4: Rotation Animation (Overshoot) ===
    lv_obj_t* box4 = lv_obj_create(scr);
    lv_obj_set_size(box4, 60, 60);
    lv_obj_set_pos(box4, 20, 200);
    lv_obj_set_style_bg_color(box4, lv_color_hex(0xf39c12), 0);
    lv_obj_set_style_radius(box4, 4, 0);
    
    lv_obj_t* label4 = lv_label_create(scr);
    lv_label_set_text(label4, "Rotate 360°");
    lv_obj_set_style_text_color(label4, lv_color_white(), 0);
    lv_obj_align_to(label4, box4, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    LVAnimation* anim4 = new LVAnimation(box4);
    anim4->setProperty(LVAnimation::Property::Rotation)
         .setValues(0, 3600)  // 0-360 degrees (x10)
         .setDuration(2000)
         .setPath(LVAnimation::Path::Linear)
         .setRepeatCount(-1);
    anim4->start();
    
    // === Demo 5: Y Position with Callbacks ===
    lv_obj_t* box5 = lv_obj_create(scr);
    lv_obj_set_size(box5, 60, 60);
    lv_obj_set_pos(box5, 200, 200);
    lv_obj_set_style_bg_color(box5, lv_color_hex(0x9b59b6), 0);
    lv_obj_set_style_radius(box5, 30, 0);
    
    lv_obj_t* label5 = lv_label_create(scr);
    lv_label_set_text(label5, "Bounce Y");
    lv_obj_set_style_text_color(label5, lv_color_white(), 0);
    lv_obj_align_to(label5, box5, LV_ALIGN_OUT_BOTTOM_MID, 0, 65);
    
    LVAnimation* anim5 = new LVAnimation(box5);
    anim5->setProperty(LVAnimation::Property::Y)
         .setValues(200, 280)
         .setDuration(600)
         .setPath(LVAnimation::Path::Bounce)
         .setRepeatCount(-1)
         .setPlayback(true)
         .onStart([]() {
             LV_LOG_INFO("Bounce animation started");
         })
         .onReady([]() {
             LV_LOG_INFO("Bounce animation cycle complete");
         });
    anim5->start();
    
    // === Demo 6: Sequential Animations ===
    lv_obj_t* box6 = lv_obj_create(scr);
    lv_obj_set_size(box6, 80, 40);
    lv_obj_set_pos(box6, 400, 200);
    lv_obj_set_style_bg_color(box6, lv_color_hex(0x1abc9c), 0);
    lv_obj_set_style_radius(box6, 20, 0);
    
    lv_obj_t* label6 = lv_label_create(box6);
    lv_label_set_text(label6, "Click!");
    lv_obj_set_style_text_color(label6, lv_color_white(), 0);
    lv_obj_center(label6);
    
    // Add click event to trigger animation
    lv_obj_add_flag(box6, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(box6, [](lv_event_t* e) {
        lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
        
        // Pulse animation
        LVAnimation* pulse = new LVAnimation(target);
        pulse->setProperty(LVAnimation::Property::Width)
             .setValues(80, 100)
             .setDuration(150)
             .setPath(LVAnimation::Path::EaseOut)
             .setRepeatCount(1)
             .setPlayback(true);
        pulse->start();
        
        LVAnimation* pulse_h = new LVAnimation(target);
        pulse_h->setProperty(LVAnimation::Property::Height)
               .setValues(40, 50)
               .setDuration(150)
               .setPath(LVAnimation::Path::EaseOut)
               .setRepeatCount(1)
               .setPlayback(true);
        pulse_h->start();
    }, LV_EVENT_CLICKED, nullptr);
    
    lv_obj_t* label6_hint = lv_label_create(scr);
    lv_label_set_text(label6_hint, "Click to pulse");
    lv_obj_set_style_text_color(label6_hint, lv_color_white(), 0);
    lv_obj_align_to(label6_hint, box6, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    // Instructions
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, 
        "Bounce X | Grow/Shrink | Fade | Rotate | Bounce Y | Click Button");
    lv_obj_set_style_text_color(info, lv_color_hex(0x95a5a6), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_14, 0);
    lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    lv_scr_load(scr);
}
