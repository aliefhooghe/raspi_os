#include <stdint.h>

#include "hardware/mmu.h"

extern void _mmu_set_ttbr0(uint32_t table_addr);
extern void _mmu_set_dacr(uint32_t dacr_value);
extern void _mmu_enable(void);

static uint32_t _page_table[4096] __attribute__((aligned(0x4000))); // 16 KB alignée

void _mmu_init_identity_page_table(void)
{
    for (int i = 0; i < 4096; i++) {
        const uint32_t page_address = i << 20;
        _page_table[i] =
            page_address |
            MMU_L1_TYPE_SECTION |
            MMU_L1_SECTION_AP_KERNEL_RW_USER_RW;
    }
}

void mmu_init(void)
{
    // initialize an identity mapping
    _mmu_init_identity_page_table();

    // set the page table physical address to TTBR0
    _mmu_set_ttbr0((uint32_t)_page_table);

    // set all domain to client mode. TODO: refine this
    _mmu_set_dacr(0x55555555);

    // enable the mmu
    _mmu_enable();
}
