


@ mapping bus to physical addresses. The arm peripheral reference manual gives us bus adresses.
@ 0x7E215054  => 0x20215054
@ 0x7E00B000  => 0x2000B000

    .section ".text"

    .globl _reset_handler
    .globl _undefined_handler
    .globl _svc_handler
    .globl _prefetch_abort
    .globl _data_abort
    .globl _reserved_handler
    .globl _irq_handler
    .globl _fiq_handler

/**
 *	Interupt Handlers
 */

// Reset
_reset_handler:

    /**
     *  Copy the interupt vector from 0x8000 (kernel) to 0x0000
     *
     *  vector size
     *   = 8 handler * (4 byte + 4 byte)
     *   = 64 bytes
     *
     */
    mov r0, #0x8000  // data source
    mov r1, #0x0000  // data destination
    ldmia r0!, {r2,r3,r4,r5,r6,r7,r8,r9} // load r2-r9 from *r0++
    stmia r1!, {r2,r3,r4,r5,r6,r7,r8,r9} // store r2-r9 to *r1++ (8 registers * 4 byte = 32 bytes)
    ldmia r0!, {r2,r3,r4,r5,r6,r7,r8,r9} // again, 32 more bytes => 64 bytes
    stmia r1!, {r2,r3,r4,r5,r6,r7,r8,r9}

    /**
     *  Setup the stacks for the different cpu modes
     *
     *     FIQ stack:   0x00001000 -> 0x00000000: 4 KB
     *     IRQ stack:   0x00002000 -> 0x00001000: 4 KB
     *     SVC stack:   0x00003000 -> 0x00002000: 4 KB  // should become bigger. TODO: CANARY test the ram
     */

    // switch to mode FIQ (Fast Interrupt Mode) / no interupt
    // PSR_FIQ_MODE = 0b10001 (0x11), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0xD1 = (0x80 | 0x40 | 0x11)
    mov r0, #0xD1
    msr cpsr_c, r0
    mov sp, #0x00001000

    // switch to mode IRQ / no interupt
    // PSR_IRQ_MODE = 0b10010 (0x12), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0xD2 = (0x80 | 0x40 | 0x12)
    mov r0, #0xD2
    msr cpsr_c,r0
    mov sp, #0x00002000

    // switch to supervisor mode / no interupt
    // PSR_SVC_MODE = 0b10011 (0x13), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0xD3 = (0x80 | 0x40 | 0x13)
    mov r0, 0xD3
    msr cpsr_c, r0
    mov sp, #0x00003000

    // jump to kernel entrypoint
    bl kernel_main
hang: b hang




_svc_handler:
    // svc number         : from lr (not used for now)
    // syscall number     : r0
    // syscall arguments  : r1 = arg0, r2 = arg1

    // save user registers on the stack
    stmfd sp!, {r1-r12, lr}

    mov r3, r2 // get arg1 to r3
    mov r2, r1 // get arg0 to r2

    // not used anymore: Get svc number in r0
    // ldr r0, [lr, #-4]
    // and r0, r0, #0xFF

    // Get program status in r1
    // When an exception (like SVC) occurs, the processor:
    // - Saves processor state (CPSR) to SPSR.
    // - Changes the current mode to supervisor mode (or another preferred mode).
    // mrs  : Move Register from Status register
    // spsr : Saved Program Status Register (mode, flags)
    mrs r1, spsr

    // call C interupt handler
    //  r0 = syscall_num
    //  r1 = program_status
    //  r2 = arg0
    //  r3 = arg1
    bl kernel_syscall_handler

    // restore user registers
    ldmfd sp!, {r1-r12, lr}

    // note: here r1-r12, lr are restored from the previous values
    // this could be from another place

    // return
    movs pc, lr

_irq_handler:
    // timers, etc.
    // pc = lr - #4 (lr = return adress): return from interupt
    stmfd sp!, {r0-r12, lr}
    bl irq_handler
    ldmfd sp!, {r0-r12, lr}
    subs pc, lr, #4

// Unhandled interuptions
_undefined_handler:
_prefetch_abort:
_data_abort:
_reserved_handler:
_fiq_handler:
    // Default handler: hang
    b .

    .globl syscall
syscall:
    //  syscall number     : r0
    //  syscall arguments  : r1 = arg0, r2 = arg1
    svc #0
    bx lr
