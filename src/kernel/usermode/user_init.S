

    .section ".text"

    .globl start_usermode
start_usermode:
    // r0: function
    // r1: stack

    // warning: if we get back to caller: save registers

    // switch to mode USER / no interupt
    // PSR_USER_MODE = 0b10010 (0x10), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0x50 = (0x40 | 0x80 | 0x10)
    mov r2, #0xD0
    msr cpsr_c, r2

    // set the user stack
    mov sp, r1

    // Call C code in usermode
    bx r0
