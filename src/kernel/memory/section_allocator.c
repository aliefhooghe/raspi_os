
#include <stdint.h>
#include <stddef.h>

#include "bitfield.h"
#include "section_allocator.h"
#include "hardware/mini_uart.h"
#include "hardware/mmu.h"

#include "lib/str.h"

// 8 * 8 = 64 bits. Can manage up to 64 sections = 64 Mb
#define SECTION_ALLOCATOR_BITFIELD_COUNT 8u 

typedef struct {
    uint8_t alloc_bitfields[SECTION_ALLOCATOR_BITFIELD_COUNT];
    uint32_t sections_base;
} section_allocator_t;

static section_allocator_t _section_allocator;

void section_allocator_init(uint32_t sections_base)
{
    mini_uart_kernel_log("section allocator: init: base=%x", sections_base);
    _memset(&_section_allocator, 0, sizeof(section_allocator_t));
    _section_allocator.sections_base = sections_base;
}

void *section_allocator_alloc(void)
{
    const int32_t section_index = bitfield_acquire_first(
        _section_allocator.alloc_bitfields,
        SECTION_ALLOCATOR_BITFIELD_COUNT);
    if (section_index < 0) {
        return NULL;
    }
    else {
        void *const section = (void *)(_section_allocator.sections_base + section_index * MMU_SECTION_SIZE);
        mini_uart_kernel_log(
            "section allocator: alloc section @ %x",
            section);
        return section;
    }
}

void section_allocator_free(void *section)
{
    mini_uart_kernel_log(
        "section allocator: free section @ %x",
        section);
    const uint32_t address = (uint32_t)section;
    const uint32_t section_offset = address - _section_allocator.sections_base;
    const uint32_t section_index = section_offset >> 20;
    bitfield_clear(
        _section_allocator.alloc_bitfields,
        SECTION_ALLOCATOR_BITFIELD_COUNT,
        section_index);
}
