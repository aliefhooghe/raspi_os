    .section ".text"

    .include "src/kernel/hardware/cpu_modes.inc"

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
     *  DANGER: TODO: the stacks are too low
     *
     *     FIQ stack:   0x00001000 -> 0x00000000: 4 KB
     *     IRQ stack:   0x00002000 -> 0x00001000: 4 KB
     *     SVC stack:   0x00006000 -> 0x00002000: 16 KB  // TODO: CANARY test the stack
     *     ABORT st.:   0x00007000 -> 0x00006000: 4 KB
     */

    // irq/fiq are disabled at startup

    // setup stack svc stack
    SET_CPU_MODE #CPU_MODE_SVC
    mov sp, #0x00006000

    // setup stack for FIQ/IRQ/ABORT modes
    SET_CPU_MODE #CPU_MODE_FIQ
    mov sp, #0x00001000

    SET_CPU_MODE #CPU_MODE_IRQ
    mov sp, #0x00002000

    SET_CPU_MODE #CPU_MODE_ABORT
    mov sp, #0x00007000

    // run kernel in svc mode
    SET_CPU_MODE #CPU_MODE_SVC
 
    // jump to kernel entrypoint
    bl kernel_main

    // TODO: display a fatal error
hang: b hang

// Software interupts
_svc_handler:
    // irqs are disabled for us, but not FIQ !

    // svc number         : from lr (not used for now)
    // syscall number     : r0
    // syscall arguments  : r1 = arg0, r2 = arg1, r3 = arg2 -> already good for kernel handler

    // I - save the task context on the stack:

    // I.1 - save r1-r12 and lr_svc registers on the stack
    stmfd sp!, {r1-r12, lr}

    // may be usefull later: retrieve the svcnum
    // record SVC number in r7
    // ldr r7, [lr, #-4]
    // and r7, r7, #0x00FFFFFF

    // I.2 - save program status on the stack
    // When an exception (like SVC) occurs, the processor:
    // - Saves processor state (CPSR) to SPSR.
    // - Changes the current mode to supervisor mode (or another preferred mode).
    // mrs  : Move Register from Status register
    // spsr : Saved Program Status Register (mode, flags)
    mrs r4, spsr
    push { r4 }

    // I.3 - save the **user mode** stack pointer/lr
    // in system mode we read user lr/sp
    // switch to the system cpu mode (0x1F)
    SET_CPU_MODE #CPU_MODE_SYS

    mov r4, lr              // save lr_svc
    mov r5, sp              // save sp

    // switch back to the SVC cpu mode (0x13)
    SET_CPU_MODE #CPU_MODE_SVC

    push { r4-r5 }          // push user mode lr then stack pointer

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
    // @see task_context.h: sizeof(task_context_t) == 68
    //      r0 is was already pop: 68 - 4 = 64
    add sp, sp, #64

    // III.2 call scheduler: get the task context to be restored
    // also restore the translation table
    bl scheduler_switch_task

    // restore the next task context
    b __scheduler_set_task_context  // r0=process

_irq_handler:
    // irqs are disabled for us, but not FIQ !

    // timers, etc.
    // pc = lr - #4 (lr = return adress): return from interupt

    // Push current stack context
    // save r1_r12 and lr_irq registers on the stack
    sub lr, lr, #4
    stmfd sp!, {r1-r12, lr}

    // save spsr on the stack
    mrs r1, spsr
    push { r1 }

    // save lr user, sp
    SET_CPU_MODE #CPU_MODE_SYS
    mov r4, lr              // save lr
    mov r5, sp              // save sp
    SET_CPU_MODE #CPU_MODE_IRQ
    push { r4-r5 }          // push user mode lr then stack pointer

    push { r0 }             // push user r0 on the stack
    // ----

    bl kernel_restore_translation_table

    // save current process context
    mov r0, sp
    bl scheduler_save_current_context
    add sp, sp, #68   // sizeof(context)

    // handle interupt
    bl irq_handler

    // switch to next process
    bl scheduler_switch_task
    b __scheduler_set_task_context  // r0=process

_fiq_handler:
    subs pc, lr, #4

_data_abort:

    // read saved status register
    mrs r0, spsr

    // TODO: what if the kernel is faulty ?????????
    // switch to svc mode, throw away current process state
    SET_CPU_MODE #CPU_MODE_SVC
    push { r0 }

    // restore kernel translation table
    bl kernel_restore_translation_table

    // error handling
    pop { r0 }
    bl scheduler_data_exception

    // switch to next stack
    bl scheduler_switch_task
    b __scheduler_set_task_context  // r0=process


// Unhandled interuptions
_undefined_handler:
_prefetch_abort:
_reserved_handler:

    SET_CPU_MODE #CPU_MODE_SVC
    bl kernel_restore_translation_table

    // Default handler: fatal error
    bl kernel_unhandled_interupt_fatal_error
    b .
