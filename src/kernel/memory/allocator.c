
#include "allocator.h"

#include <stdint.h>

void memory_allocator_init(
    memory_allocator_t *allocator,
    uint32_t base,
    uint32_t size)
{
    allocator->limit = base;
    allocator->base = base;
    allocator->size = size;
}

void *memory_allocator_alloc(memory_allocator_t *allocator, size_t size)
{
    const uint32_t max_limit = allocator->base + allocator->size;
    const uint32_t new_limit = allocator->limit + size;
    if (new_limit > max_limit)
    {
        return NULL;
    }
    else
    {
        const uint32_t result = allocator->limit;
        allocator->limit = new_limit;
        return (void*)result;
    }
}

void memory_allocator_free(memory_allocator_t *allocator, void *chunk)
{
    (void)allocator,
    (void)chunk;
}
