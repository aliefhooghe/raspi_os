
    .section ".text"

// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42 - machine id
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main

/**
 * Interupt vector (entry point)
 * Note:
 *	- The ARM processor begins by executing the instruction located at the address of the reset vector.
 *  - Using ldr ensure the handler address are absolute
 */
    .globl _start
_start:
_interupt_vector:
    ldr pc, reset_handler
    ldr pc, undefined_handler
    ldr pc, svc_handler
    ldr pc, prefetch_abort
    ldr pc, data_abort
    ldr pc, reserved_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler
reset_handler:      .word _reset_handler     // Reset
undefined_handler:  .word _undefined_handler // Undefined Instruction
svc_handler:        .word _svc_handler       // Supervisor Call (SVC)  (anciennement SWI: software interuption)
prefetch_abort:     .word _prefetch_abort    // Prefetch Abort
data_abort:         .word _data_abort        // Data Abort
reserved_handler:   .word _reserved_handler  // Reserved
irq_handler:        .word _irq_handler       // IRQ
fiq_handler:        .word _fiq_handler       // FIQ
