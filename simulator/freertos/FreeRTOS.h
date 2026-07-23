#pragma once
#include <stdint.h>

typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) (ms)
#define pdTICKS_TO_MS(ticks) (ticks)
static inline void vTaskDelay(TickType_t ticks) { (void)ticks; }
