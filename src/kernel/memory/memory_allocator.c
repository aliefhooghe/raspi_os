
#include <stdint.h>

#include "hardware/mmu.h"

#include "kernel.h"

#include "lib/str.h"

#include "memory_allocator.h"
#include "section_allocator.h"

typedef struct {
    uint8_t *memory_cursor;
    uint8_t *section_end;
} kernel_memory_allocator ;


static kernel_memory_allocator _kernel_memory_allocator;

void memory_allocator_init(void)
{
    _memset(&_kernel_memory_allocator, 0u, sizeof(kernel_memory_allocator));
    uint8_t *const section = (uint8_t*)section_allocator_alloc();
    if (section == NULL)
        kernel_fatal_error("memory allocator: failed to allocate a section");
    _kernel_memory_allocator.memory_cursor = section;
    _kernel_memory_allocator.section_end = section + MMU_SECTION_SIZE;
}

void *memory_alloc(size_t size)
{
    uint8_t *new_cursor = (uint8_t*)
        (((uint32_t)_kernel_memory_allocator.memory_cursor + size + 0x3u) & ~0x3);
    if (new_cursor > _kernel_memory_allocator.section_end)
        return NULL;

    const uint8_t *mem = _kernel_memory_allocator.memory_cursor;
    _kernel_memory_allocator.memory_cursor = new_cursor;
    return (void*)mem;
}

void memory_free(void *ptr)
{
    (void)ptr;
}
