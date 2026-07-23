#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

lv_display_t * sdl_hal_init(int32_t w, int32_t h);

#ifdef __cplusplus
}
#endif
