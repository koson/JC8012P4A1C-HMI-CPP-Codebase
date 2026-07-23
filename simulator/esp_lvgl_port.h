#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool lvgl_port_lock(uint32_t timeout_ms) { (void)timeout_ms; return true; }
static inline void lvgl_port_unlock(void) {}

#ifdef __cplusplus
}
#endif
