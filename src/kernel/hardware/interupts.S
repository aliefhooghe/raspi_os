


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
     *     SVC stack:   0x00003000 -> 0x00002000: 4 KB  // should become bigger. TODO: CANARY test the stack
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
    // syscall arguments  : r1 = arg0, r2 = arg1, r3 = arg2 -> already good for kernel handler

    // I - save the task context on the stack:

    // I.1 - save user registers on the stack
    stmfd sp!, {r1-r12, lr}

    // I.2 - save program status on the stack
    // When an exception (like SVC) occurs, the processor:
    // - Saves processor state (CPSR) to SPSR.
    // - Changes the current mode to supervisor mode (or another preferred mode).
    // mrs  : Move Register from Status register
    // spsr : Saved Program Status Register (mode, flags)
    mrs r4, spsr
    push { r4 }

    // I.3 - save the **user mode** stack pointer
    mov r4, #0x1F            // switch to the system cpu mode (0x1F)
    msr cpsr_c, r4

    mov r5, sp              // save sp

    mov r4, #0x13           // switch to the SVC cpu mode (0x13)
    msr cpsr_c, r4

    push { r5 }             // push user mode stack pointer

    // save syscall number and parameters
    push {r0-r3}

    // restore kernel translation table
    bl kernel_restore_translation_table

    // store the current task context
    mov r0, sp
    add r0, #12  //  skip r1-r3
    bl scheduler_save_current_context

    // restore syscall number and parameters
    pop {r0-r3}

    // II - call the C syscall handler
    //  r0 = syscall_num
    //  r1 = arg0
    //  r2 = arg1
    //  r3 = arg2
    bl kernel_syscall_handler

    // III - schedule the next task

    // III.1 restore supervisor stack pointer: free the saved context
    // task_context = r1-r12, lr, spsr = 4 * (12 + 2) = 56 bytes
    add sp, sp, #56

    // III.2 call scheduler: get the task context to be restored
    bl scheduler_switch_task

    // restore the next task context
    b __set_task_context

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
    // Default handler: fatal error
    bl kernel_unhandled_interupt_fatal_error
    b .
