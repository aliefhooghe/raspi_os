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

void translation_table_add_kernel_mapping(
    uint32_t *translation_table)
{
    // map IO registers for the kernel
    // TODO: set le TEX et cie car nous ne voulons pas de mise en cache
    // ainsi que les bits C et B
    // et de réordonement des accès mémoire sur les mmios
    translation_table_add_identity_mapping(translation_table,
        IO_REG_START, IO_REG_END,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_NONE);

    // map the kernel memory on identity for both user and kernel
    translation_table_add_identity_mapping(translation_table,
        0x00000000u, 0x04800000,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);
}
