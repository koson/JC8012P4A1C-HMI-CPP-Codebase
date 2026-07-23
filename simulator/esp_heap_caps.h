#pragma once
#include <stdlib.h>
#include <stdint.h>

#define MALLOC_CAP_SPIRAM 0
#define MALLOC_CAP_DEFAULT 0
#define MALLOC_CAP_INTERNAL 0
#define MALLOC_CAP_8BIT 0

static inline void *heap_caps_malloc(size_t size, uint32_t caps) { (void)caps; return malloc(size); }
static inline void *heap_caps_calloc(size_t n, size_t size, uint32_t caps) { (void)caps; return calloc(n, size); }
static inline void heap_caps_free(void *ptr) { free(ptr); }
static inline size_t heap_caps_get_free_size(uint32_t caps) { (void)caps; return 1024 * 1024; }
static inline size_t heap_caps_get_largest_free_block(uint32_t caps) { (void)caps; return 512 * 1024; }
