#ifndef SATAN_MMU_H_
#define SATAN_MMU_H_

// mmu level 1 table entry type
#define MMU_L1_TYPE_MASK          0x03u  //
#define MMU_L1_TYPE_FAULT         0x00u  // Invalid
#define MMU_L1_TYPE_PAGE_TABLE    0x01u  // Level 2 table pointer
#define MMU_L1_TYPE_SECTION       0x02u  // Section

#define MMU_L1_SECTION_AP_MASK                 0x0000C000u  // kernel mode -  user mode
#define MMU_L1_SECTION_AP_KERNEL_RW_USER_NONE  0x00000000u  // read/write  *  none
#define MMU_L1_SECTION_AP_KERNEL_RW_USER_R     0x00004000u  // read/write  *  read only
#define MMU_L1_SECTION_AP_KERNEL_RW_USER_RW    0x00008000u  // read/write  *  read/write
#define MMU_L1_SECTION_AP_KERNEL_R_USER_NONE   0x0000C000u  // read only   *  none


void mmu_init_identity_page_table(void);

#endif
