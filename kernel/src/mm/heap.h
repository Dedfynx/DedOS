#pragma once

#include <stddef.h>

#define SLAB_32 32
#define SLAB_64 64
#define SLAB_128 128
#define SLAB_256 256
#define SLAB_512 512
#define SLAB_1024 1024

void heap_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t count, size_t size);
void kfree(void* ptr);
