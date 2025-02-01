
#include "allocator.h"

#include "lib/str.h"

void memory_allocator_init(memory_allocator_t *allocator)
{
    _memset(&allocator, 0, sizeof(memory_allocator_t));
}

void *memory_allocator_alloc(memory_allocator_t *allocator, size_t size)
{
    (void)allocator,
    (void)size;
    return NULL;
}

void memory_allocator_free(memory_allocator_t *allocator, void *chunk)
{
    (void)allocator,
    (void)chunk;
}
