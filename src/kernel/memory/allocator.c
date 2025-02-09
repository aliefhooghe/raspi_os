#include <stdint.h>

#include "allocator.h"

memory_allocator_t _allocator;


void memory_allocator_init(
    uint32_t base,
    uint32_t limit)
{
    _allocator.base = base;
    _allocator.limit = limit;
    _allocator.cursor = base;
}

void *mem_alloc(size_t size)
{
    const uint32_t new_cursor = _allocator.cursor + size;
    if (new_cursor > _allocator.limit)
    {
        return NULL;
    }
    else
    {
        const uint32_t result = _allocator.cursor;
        _allocator.cursor = new_cursor;
        return (void*)result;
    }
}

void mem_free(void *chunk)
{
    (void)chunk;
}
