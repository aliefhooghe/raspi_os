#ifndef SATAN_MEMORY_ALLOCATOR_H_
#define SATAN_MEMORY_ALLOCATOR_H_

#include <stddef.h>

void memory_allocator_init(void);
void *memory_alloc(size_t size);
void memory_free(void *ptr);

#endif
