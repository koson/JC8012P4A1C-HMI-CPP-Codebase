#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool bsp_display_lock(uint32_t timeout_ms) {
    (void)timeout_ms;
    return true;
}

static inline void bsp_display_unlock(void) {
}

#ifdef __cplusplus
}
#endif
