
#include "allocator.h"

#include <stdint.h>

void memory_allocator_init(
    memory_allocator_t *allocator,
    uint32_t base,
    uint32_t limit)
{
    allocator->base = base;
    allocator->limit = limit;
    allocator->cursor = base;
}

void *memory_allocator_alloc(memory_allocator_t *allocator, size_t size)
{
    const uint32_t new_cursor = allocator->cursor + size;
    if (new_cursor > allocator->limit)
    {
        return NULL;
    }
    else
    {
        const uint32_t result = allocator->cursor;
        allocator->cursor = new_cursor;
        return (void*)result;
    }
}

void memory_allocator_free(memory_allocator_t *allocator, void *chunk)
{
    (void)allocator,
    (void)chunk;
}
