#ifndef SATAN_MMU_H_
#define SATAN_MMU_H_

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

/**
 *  Enable the Memory Management Unit
 */
void mmu_init(void);

#endif
