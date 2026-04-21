#include "bsp/esp-bsp.h"
#include "bsp/display.h"



void test_Font(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *label1 = lv_label_create(scr);
    lv_obj_t *label2 = lv_label_create(scr);
    lv_label_set_text(label1, "Hello World");
    // label set font size และทำตัวหนา
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(label1, lv_color_hex(0xFF0000), 0);
   
    lv_obj_set_pos(label1, 100, 50);
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 250, 50);

    lv_label_set_text_fmt(label2, "Value: %d", 100);  // Format แบบ printf
    lv_obj_set_pos(label2, 100, 100);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label2, lv_color_hex(0x008888), 0);
}


void test_Button(void)
{
    lv_obj_t *parent = lv_scr_act();
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_t *label = lv_label_create(btn);
    lv_obj_set_pos(btn, 100, 150);
    lv_label_set_text(label, "Click Me");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x004444), 0);
    lv_obj_set_size(btn, 300, 100);
}

void test_Line(void)
{
    lv_obj_t *parent = lv_scr_act();
    lv_obj_t *line = lv_line_create(parent);

    // กำหนดจุด
    static lv_point_precise_t points[2];
    points[0].x = 0;   points[0].y = 0;
    points[1].x = 100; points[1].y = 100;

    lv_line_set_points(line, points, 2);
    lv_obj_set_style_line_width(line, 2, 0);
    lv_obj_set_style_line_color(line, lv_color_hex(0x00FF00), 0);

}

void arc_example_v9(void)
{
    /* สร้าง arc บนหน้าจอปัจจุบัน */
    lv_obj_t * arc = lv_arc_create(lv_screen_active());

    /* ตั้งค่าขนาด */
    lv_obj_set_size(arc, 150, 150);

    /* จัดตำแหน่งกลางจอ */
    lv_obj_center(arc);

    /* ตั้งค่ามุมพื้นหลัง (background arc) */
    lv_arc_set_bg_angles(arc, 0, 360);

    /* ตั้งค่ามุม arc ที่แสดง */
    lv_arc_set_angles(arc, 0, 270);

    /* ตั้งค่าช่วงค่า (ใช้เป็น progress ได้) */
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, 75);

    /* เปิดโหมดหมุนอิสระ (ถ้าต้องการให้ผู้ใช้หมุนเอง) */
    lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
}
