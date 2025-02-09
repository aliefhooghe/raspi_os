#include <stdint.h>

#include "hardware/mmu.h"
#include "hardware/io_registers.h"
#include "lib/str.h"

extern void _mmu_set_ttbr0(uint32_t table_addr);
extern void _mmu_set_dacr(uint32_t dacr_value);
extern void _mmu_enable(void);


static uint32_t _page_table[MMU_L1_ENTRY_COUNT] __attribute__((aligned(MMU_L1_TABLE_ALIGN))); // 16 KB alignée


void _mmu_identity_mapping(
    uint32_t *l1_table,
    uint32_t start_address,
    uint32_t end_address,
    uint32_t mem_protection)
{
    for (uint32_t section_address = start_address;
        section_address < end_address;
        section_address += MMU_SECTION_SIZE)
    {
        const uint32_t section_index = section_address >> 20;
        l1_table[section_index] = section_address  |
            mem_protection |
            MMU_L1_TYPE_SECTION;
    }
}

void mmu_init(void)
{
    // initialize an identity mapping
    _memset(_page_table, 0, sizeof(uint32_t) * 4096);

    // map IO registers for the kernel
    // TODO: set le TEX et cie car nous ne voulons pas de mise en cache
    // ainsi que les bits C et B
    // et de réordonement des accès mémoire sur les mmios
    _mmu_identity_mapping(_page_table,
        IO_REG_START, IO_REG_END,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_NONE);

    // map the kernel memory on identity for both user and kernel
    _mmu_identity_mapping(_page_table,
        0x00000000u, 0x00800000,
        MMU_L1_SECTION_AP_KERNEL_RW_USER_RW);

    // set the page table physical address to TTBR0
    _mmu_set_ttbr0((uint32_t)_page_table);

    // set all domain to client mode. TODO: refine this
    _mmu_set_dacr(0x55555555);

    // enable the mmu
    _mmu_enable();
}
