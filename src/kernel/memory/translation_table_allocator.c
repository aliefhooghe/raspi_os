#include "bitfield.h"

#include "hardware/mmu.h"
#include "lib/str.h"



#define TRANSLATION_TABLE_ALLOCATOR_BITFIELD_COUNT 8u // 8 * 8 = 64 translation_tables

/**
 *
 */
typedef struct {
    uint8_t alloc_bitfields[TRANSLATION_TABLE_ALLOCATOR_BITFIELD_COUNT];
    uint32_t *memory_section_base;
} translation_table_allocator_t;


translation_table_allocator_t _translation_table_allocator;


void translation_table_allocator_init(void *memory_section)
{
    _memset(&_translation_table_allocator, 0, sizeof(translation_table_allocator_t));
    _translation_table_allocator.memory_section_base = (uint32_t*)memory_section;
}

uint32_t *translation_table_allocator_alloc(void)
{
    const int32_t table_index = bitfield_acquire_first(
        _translation_table_allocator.alloc_bitfields,
        TRANSLATION_TABLE_ALLOCATOR_BITFIELD_COUNT);
    if (table_index < 0)
        return NULL;
    else
        return _translation_table_allocator.memory_section_base + table_index * MMU_L1_ENTRY_COUNT;
}

void translation_table_allocator_free(uint32_t *translation_table)
{
    const uint32_t table_offset = translation_table - _translation_table_allocator.memory_section_base;
    const uint32_t table_index = table_offset / MMU_L1_ENTRY_COUNT;
    bitfield_clear(_translation_table_allocator.alloc_bitfields, table_index);
}
