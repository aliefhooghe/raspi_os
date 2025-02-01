#ifndef SATAN_MEMORY_ALLOCATOR_H_
#define SATAN_MEMORY_ALLOCATOR_H_

#include <stddef.h>

typedef struct {
    int dummy;
} memory_allocator_t;

void memory_allocator_init(memory_allocator_t *allocator);
void *memory_allocator_alloc(memory_allocator_t *allocator, size_t size);
void memory_allocator_free(memory_allocator_t *allocator, void *chunk);

#endif
