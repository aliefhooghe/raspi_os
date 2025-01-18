
// To keep this in the first portion of the binary.
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
    b _svc_handler        // Supervisor Call (SVC)
    b _prefetch_abort     // Prefetch Abort
    b _data_abort         // Data Abort
    b _reserved_handler   // Reserved
    b _irq_handler        // IRQ
    b _fiq_handler        // FIQ


/**
 *	Interupt Handlers
 */

// Reset
_reset_handler:
	// Setup the stack.
	mov sp, #0x8000

	// Clear out bss.
	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f

1:
	// store multiple at r4.
	stmia r4!, {r5-r8}

	// If we are still below bss_end, loop.
2:
	cmp r4, r9
	blo 1b

	// Call kernel_main
	ldr r3, =kernel_main
	blx r3

	// halt
halt:
	wfe
	b halt

// Unhandled interuptions
_undefined_handler:
_svc_handler:
_prefetch_abort:
_data_abort:
_reserved_handler:
_irq_handler:
_fiq_handler:
    // Default handler: hang
    b .
