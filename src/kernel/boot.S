
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
 */
.globl _start
_start:
_interupt_vector:
    b _reset_handler      // Reset
    b _undefined_handler  // Undefined Instruction
    b _svc_handler        // Supervisor Call (SVC)  (anciennement SWI: software interuption)
    b _prefetch_abort     // Prefetch Abort
    b _data_abort         // Data Abort
    b _reserved_handler   // Reserved
    b _irq_handler        // IRQ
    b _fiq_handler        // FIQ
