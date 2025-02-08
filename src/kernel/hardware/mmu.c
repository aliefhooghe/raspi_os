#include <stdint.h>

#include "hardware/mmu.h"

uint32_t page_table[4096] __attribute__((aligned(0x4000))); // 16 KB alignée

void mmu_init_identity_page_table(void)
{
    for (int i = 0; i < 4096; i++) {
        const uint32_t page_address = i << 20;
        page_table[i] =
            page_address |
            MMU_L1_TYPE_SECTION |
            MMU_L1_SECTION_AP_KERNEL_RW_USER_RW;
    }
}
