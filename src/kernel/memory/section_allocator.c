
#include <stdint.h>
#include <stddef.h>

#include "section_allocator.h"
#include "hardware/mmu.h"

#include "lib/str.h"


#define SECTION_ALLOCATOR_BITFIELD_COUNT 2u // 2 * 32 = 64 bits. Can manage up to 64 sections = 64 Mb

typedef struct {
    uint32_t alloc_bitfield[SECTION_ALLOCATOR_BITFIELD_COUNT];
    uint32_t sections_base;
} section_allocator_t;

static section_allocator_t _section_allocator;

static void *_get_section_address(uint32_t section_index)
{
    return (void *)(_section_allocator.sections_base + section_index * MMU_SECTION_SIZE);
}

static uint32_t _get_section_index(void *section_address)
{
    const uint32_t address = (uint32_t)section_address;
    const uint32_t section_offset = address - _section_allocator.sections_base;
    return section_offset >> 20; // 1 sectop, = 0x100000 bytes
}

void section_allocator_init(uint32_t sections_base)
{
    _memset(&_section_allocator, 0, sizeof(section_allocator_t));
    _section_allocator.sections_base = sections_base;
}

void *section_allocator_alloc_section(void)
{
    for (uint32_t bitfield_index = 0u;
        bitfield_index < SECTION_ALLOCATOR_BITFIELD_COUNT;
        bitfield_index++)
    {
        uint32_t *bitfield = &_section_allocator.alloc_bitfield[bitfield_index];
        if (*bitfield == 0xFFFFFFFFu)
            continue;
        for (uint32_t bit_index = 0u; bit_index < 32; bit_index++)
        {
            if (0 == ((1 << bit_index) & *bitfield))
            {
                // set the alloc bit
                *bitfield |= (1u << bit_index);

                // return the actual section address
                const uint32_t section_index = bit_index + 32 * bitfield_index;
                return _get_section_address(section_index);
            }
        }
    }

    return NULL;
}

void section_allocator_free(void *section)
{
    const uint32_t section_index = _get_section_index(section);
    const uint32_t bitfield_index = section_index >> 5u;
    const uint32_t bit_index = section_index & 0x1fu;

    if (bitfield_index >= SECTION_ALLOCATOR_BITFIELD_COUNT)
        return; // TODO: handle error

    _section_allocator.alloc_bitfield[bitfield_index] &= ~(1u << bit_index);
}
