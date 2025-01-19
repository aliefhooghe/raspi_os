

    .section ".text"

    .globl start_usermode
start_usermode:

    // warning: if we get back to caller: save registers

    // switch to mode USER / no FIQ
    // PSR_USER_MODE = 0b10010 (0x10), PSR_FIQ_DIS = 0x40
    // 0x50 = (0x40 | 0x10)
    mov r1, #0x50
    msr cpsr_c, r1
    mov sp, r0

    // Call C code in usermode
    bl user_init

    // Hang
hang: b hang
