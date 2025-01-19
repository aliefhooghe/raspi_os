

    .section ".text"

    .globl start_usermode
start_usermode:
    // switch to mode USER / no FIQ
    // PSR_USER_MODE = 0b10010 (0x10), PSR_FIQ_DIS = 0x40
    // 0x50 = (0x40 | 0x10)
    mov r0, #0x50
    msr cpsr_c, r0
    mov sp, 0x00800000

    // Call C code in usermode
    bl user_init

    // Hang
hang: b hang
