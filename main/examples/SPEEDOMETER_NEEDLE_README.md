# 🏎️ Speedometer with Image Needle

ตัวอย่างการสร้าง Speedometer ที่ใช้รูปเข็มจริงแทนการเส้น โดยใช้ภาพเข็มจาก LVGL official demo

## ✨ Features

- **🎯 Real Needle Image**: ใช้ภาพเข็มจริงจาก LVGL demo
- **🌈 Color-coded Speed Zones**: 
  - 🟢 Green (0-60 km/h) - Safe zone
  - 🟡 Orange (60-120 km/h) - Caution zone  
  - 🔴 Red (120-200 km/h) - Danger zone
- **🎮 Interactive Controls**: Slider และ preset buttons สำหรับควบคุมความเร็ว
- **✨ Smooth Animation**: เข็มหมุนแบบ smooth ด้วย LVGL animation
- **📊 Scale Sections**: แสดงโซนสีบนหน้าปัด
- **🎨 Modern UI**: ดีไซน์ทันสมัยด้วยสีสันสวยงาม

## 📁 Files Structure

```
examples/
├── speedometer_with_needle.cpp      # คลาสหลัก SpeedometerNeedleDemo
├── speedometer_needle_test.cpp      # Test application
├── img_demo_widgets_needle.c        # Needle image data (copied from LVGL)
└── img_demo_widgets_needle.h        # Header file for needle image
```

## 🚀 How to Use

### 1. Build และ Flash
```bash
cd LVGL_CPP_DEV
idf build
idf flash monitor
```

### 2. Controls
- **🎚️ Slider**: ลากเพื่อปรับความเร็ว 0-200 km/h
- **🔘 Preset Buttons**: คลิกปุ่ม 0, 30, 60, 90, 120, 160, MAX
- **📱 Real-time Display**: ดูความเร็วและโซนปัจจุบันที่หน้าจอ

### 3. Speed Zones
| Zone | Speed | Color | Description |
|------|-------|-------|-------------|
| 🟢 Safe | 0-60 km/h | Green | ขับปลอดภัย |
| 🟡 Caution | 60-120 km/h | Orange | ระวังความเร็ว |
| 🔴 Danger | 120-200+ km/h | Red | อันตราย! |

## 🔧 Technical Implementation

### Scale Configuration
```cpp
lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
lv_scale_set_range(scale, 0, 200);           // 0-200 km/h
lv_scale_set_angle_range(scale, 270);        // 270° span
lv_scale_set_rotation(scale, 135);           // Start position
```

### Needle Setup
```cpp
needle = lv_image_create(scale);
lv_image_set_src(needle, &img_demo_widgets_needle);
lv_image_set_pivot(needle, 3, 4);            // Pivot point
lv_scale_set_image_needle_value(scale, needle, speed);
```

### Color-coded Sections
```cpp
// Green zone (0-60 km/h)
lv_obj_t* section1 = lv_scale_add_section(scale);
lv_scale_set_section_range(scale, section1, 0, 60);
lv_style_set_arc_color(&green_style, lv_color_hex(0x2ECC71));
```

## 🎨 Needle Image Details

- **Original Source**: `managed_components/lvgl__lvgl/demos/widgets/assets/`
- **Image Size**: 100 x 9 pixels
- **Format**: ARGB8888 (with transparency)
- **Pivot Point**: (3, 4) near the base of needle

## 📋 Integration Example

```cpp
#include "examples/speedometer_with_needle.cpp"

// In your main function
extern "C" void run_speedometer_needle_test();

void app_main() {
    // Initialize display...
    run_speedometer_needle_test();
}
```

## 🔍 Debug Information

Monitor output จะแสดง:
```
I (xxx) SPEEDOMETER_NEEDLE: ✅ Speedometer with needle created
I (xxx) SPEEDOMETER_NEEDLE: Needle created and positioned  
I (xxx) SPEEDOMETER_NEEDLE: Speed updated: 85 km/h (CAUTION zone)
```

## 🚀 Next Steps

คุณสามารถ:
1. **ปรับแต่งสี**: เปลี่ยนโทนสีให้เข้ากับ theme
2. **เพิ่ม Animation**: ใส่ animation เมื่อเปลี่ยนโซน
3. **เพิ่มเสียง**: เพิ่มเสียงเตือนเมื่อเข้าโซนอันตราย
4. **สร้างเข็มแบบกำหนดเอง**: ใช้รูปเข็มของคุณเอง

---

🎯 **Ready to test!** ลอง build และดูผลลัพธ์ที่สวยงามกัน! 🏁