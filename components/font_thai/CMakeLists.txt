idf_component_register(
    SRCS
        "src/th_niramit_16.c"
        "src/th_niramit_20.c"
        "src/th_niramit_24.c"
        "src/th_niramit_32.c"
        "src/th_niramit_40.c"
        "src/th_niramit_48.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        lvgl__lvgl
)

# Generated font files check LV_LVGL_H_INCLUDE_SIMPLE to use "lvgl.h" instead of "lvgl/lvgl.h"
target_compile_definitions(__idf_font_thai PRIVATE LV_LVGL_H_INCLUDE_SIMPLE=1)

