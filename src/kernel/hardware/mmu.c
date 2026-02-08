#include <stdint.h>

#include "hardware/mmu.h"
#include "kernel.h"

void *mmu_translate_virtual_address(
    uint32_t *translation_table,
    uintptr_t virtual_address)
{
    if (virtual_address == 0)
    {
        return NULL;
    }

    const uint32_t section_index = virtual_address >> 20;
    const uint32_t translation = translation_table[section_index];
    if ((translation & MMU_L1_TYPE_MASK) != MMU_L1_TYPE_SECTION)
    {
        kernel_fatal_error("mmu: translation: invalid L1 type");
    }

    const uint32_t mem_protection = (translation & MMU_L1_SECTION_AP_MASK);
    if (mem_protection != MMU_L1_SECTION_AP_KERNEL_RW_USER_RW)
    {
        kernel_fatal_error("mmu: translation: unauthorized access");
    }

    const uintptr_t section_address = translation & 0xFFF00000;
    const uint32_t section_offset = virtual_address & 0xFFFFFu;
    return (void*)(section_address | section_offset);
}

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
    const uint32_t section_address = ((uint32_t)memory_section & 0xFFF00000);
    const uint32_t section_index = virtual_section_address >> 20;
    translation_table[section_index] = section_address  |
            mem_protection |
            MMU_L1_TYPE_SECTION;
}
