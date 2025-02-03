

#include "kernel.h"
#include "allocator.h"

#include <stdint.h>


/**
 *  global kernel state
 */
extern kernel_state_t __kernel_state;


void memory_allocator_init(
    uint32_t base,
    uint32_t limit)
{
    memory_allocator_t *allocator = &__kernel_state.allocator;
    allocator->base = base;
    allocator->limit = limit;
    allocator->cursor = base;
}

void *mem_alloc(size_t size)
{
    memory_allocator_t *allocator = &__kernel_state.allocator;
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

void mem_free(void *chunk)
{
    (void)chunk;
}
