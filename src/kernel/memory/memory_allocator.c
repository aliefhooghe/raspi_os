
#include <stdint.h>

#include "hardware/mini_uart.h"
#include "hardware/mmu.h"

#include "kernel.h"

#include "lib/str.h"

#include "memory_allocator.h"
#include "section_allocator.h"

// --

typedef struct {
    size_t size;
    uint8_t data[4];
} memory_block_t;

typedef struct {
    uint8_t *memory_cursor;
    uint8_t *section_end;
} kernel_memory_allocator ;

static kernel_memory_allocator _kernel_memory_allocator;

static memory_block_t *_block_by_ptr(void *ptr)
{
    return (memory_block_t*)((uint8_t*)ptr - sizeof(size_t));
}

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
    mini_uart_kernel_log(
        "memory: alloc: size='%u'",
        size);
    uint8_t *new_cursor = (uint8_t*)
        ((
            (uint32_t)_kernel_memory_allocator.memory_cursor +
                sizeof(size_t) + size + 
                0x3u
            ) & ~0x3 // align to 4 bytes
        );

    if (new_cursor > _kernel_memory_allocator.section_end)
    {
        mini_uart_kernel_log("memory_alloc: failed allocation of %u bytes", size);
        return NULL;
    }

    memory_block_t *block = (memory_block_t*)_kernel_memory_allocator.memory_cursor;

    block->size = size;
    _kernel_memory_allocator.memory_cursor = new_cursor;
    mini_uart_kernel_log(
        "memory: allocated block @ %x",
        block->data);
    return block->data;
}

void *memory_realloc(void *mem, size_t size)
{
    const memory_block_t *old_block = _block_by_ptr(mem);

    if (size < old_block->size) {
        return (void*)old_block->data;
    }

    // initialize additional memory to zero
    void *new_mem = memory_calloc(size);
    _memcpy(new_mem, mem, old_block->size);

    return new_mem;
}

void *memory_calloc(size_t size)
{
    void *mem = memory_alloc(size);
    if (mem == NULL)
        return NULL;
    mini_uart_kernel_log(
        "memory: zero block @ %x (size = %u)",
        mem, size);
    _memset(mem, 0, size);
    mini_uart_kernel_log(
        "memory: block was zero initialized."
        );
    return mem;
}

void memory_free(void *ptr)
{
    mini_uart_kernel_log(
        "memory: free block @ %x",
        ptr);
}
