#ifndef SATAN_MEMORY_ALLOCATOR_H_
#define SATAN_MEMORY_ALLOCATOR_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t base;
    uint32_t cursor;
    uint32_t limit;
} memory_allocator_t;

void memory_allocator_init(
    uint32_t base,
    uint32_t limit);

void *mem_alloc(size_t size);
void mem_free(void *chunk);

#endif
