#ifndef SATAN_MMU_H_
#define SATAN_MMU_H_

#include <stdint.h>

#define MMU_L1_ENTRY_COUNT 0x1000u
#define MMU_L1_TABLE_ALIGN 0x4000u  // 16.0KiB aligned


// mmu level 1 table entry type
#define MMU_L1_TYPE_MASK          0x03u  //
#define MMU_L1_TYPE_FAULT         0x00u  // Unmapped entry
#define MMU_L1_TYPE_PAGE_TABLE    0x01u  // Level 2 table pointer
#define MMU_L1_TYPE_SECTION       0x02u  // Section

// mmu level 1 table section protection
#define MMU_L1_SECTION_AP_MASK                   0x00000C00u  // kernel mode -  user mode
#define MMU_L1_SECTION_AP_KERNEL_NONE_USER_NONE  0x00000000u  // read/write  *  none
#define MMU_L1_SECTION_AP_KERNEL_RW_USER_NONE    0x00000400u  // read/write  *  read only
#define MMU_L1_SECTION_AP_KERNEL_RW_USER_RO      0x00000800u  // read/write  *  read/write
#define MMU_L1_SECTION_AP_KERNEL_RW_USER_RW      0x00000C00u  // read only   *  none

//
#define MMU_SECTION_SIZE 0x00100000u

extern void mmu_set_dacr(uint32_t dacr_value);
extern void mmu_enable(void);
extern void mmu_set_translation_table(const uint32_t *table_addr);


/**
 *  Emulate the mmu logic and translate a virtual address to a physical
 *  address using a translation table
 */
void *mmu_translate_virtual_address(
    uint32_t *translation_table,
    uintptr_t virtual_addresss);

/**
 *  Add an identity mappping on the translation table on a given range with permissions.
 */
void translation_table_add_identity_mapping(
    uint32_t *translation_table,
    uint32_t start_address,
    uint32_t end_address,
    uint32_t mem_protection);

/**
 * Add a single virtual memory section to the translation table
 */
void translation_table_add_single_section(
   uint32_t *translation_table,
   void *memory_section,
   uint32_t virtual_section_address,
   uint32_t mem_protection);

#endif
