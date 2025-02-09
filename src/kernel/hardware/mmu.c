#include <stdint.h>

#include "hardware/mmu.h"
#include "hardware/io_registers.h"

void translation_table_add_identity_mapping(
    uint32_t *translation_table,
    uint32_t start_address,
    uint32_t end_address,
    uint32_t mem_protection)
{
    for (uint32_t section_address = start_address;
        section_address < end_address;
        section_address += MMU_SECTION_SIZE)
    {
        const uint32_t section_index = section_address >> 20;
        translation_table[section_index] = section_address  |
            mem_protection |
            MMU_L1_TYPE_SECTION;
    }
}

void translation_table_add_single_section(
    uint32_t *translation_table,
    void *memory_section,
    uint32_t virtual_section_address,
    uint32_t mem_protection)
{
    const uint32_t section_address = (uint32_t)memory_section;
    const uint32_t section_index = section_address >> 20;
    translation_table[section_index] = virtual_section_address  |
            mem_protection |
            MMU_L1_TYPE_SECTION;
}
