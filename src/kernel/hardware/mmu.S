    .section ".text"


    .globl _mmu_set_ttbr0
_mmu_set_ttbr0:
    // r0: ttbr0 value

    // mcr: Move to Coprocessor from ARM Register
    // p15: coprocessor 15 (CP15),
    // opcode_1: 0 unused
    // r0: source value
    // c2: in p15, c2 is the Translation Table Base Register 0 (TTBR0)
    mcr p15, 0, r0, c2, c0, 0 // Store r0 to TTBR0
    bx lr

    .globl _mmu_set_dacr
_mmu_set_dacr:
    // r0: ttbr0 value
    mcr p15, 0, r0, c3, c0, 0  // Store r0 to DACR
    bx  lr

    .globl _mmu_enable
_mmu_enable:
    // no arguments
    mrc p15, 0, r0, c1, c0, 0   // Load SCTLR to r0
    orr r0, r0, #1              // Set the MMU bit (bit 0) to 1
    mcr p15, 0, r0, c1, c0, 0   // Store to SCTLR
    bx  lr
