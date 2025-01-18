@ Reset (point d'entrée initiale).
@ Undefined instruction (pour des instructions illégales).
@ Software Interrupt (SWI) (pour les appels système).
@ Prefetch Abort (problème lors de la récupération d'une instruction).
@ Data Abort (problème lors de l'accès mémoire).
@ IRQ (Interrupt Request, matériel).
@ FIQ (Fast Interrupt Request, matériel prioritaire).



@ Sur mon code mini uart je trouve le registre AUX_MU_LSR_REG à l'adresse  alors que l'addresse physique indiqué dans la doc est



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

_svc_handler:
    #   syscall number      : from lr
    #   syscall arguments   : r0, r1
    #

    # save user registers on the stack
    stmfd sp!, {r0-r12, lr}

    mov r2, r0 // get 1st argument to r2
    mov r3, r1 // get 2nd argument to r3

    # Get syscall number in r0
    ldr r0, [lr, #-4]
    and r0, r0, #0xFF

    # Get program status in r1
    # When an exception (like SVC) occurs, the processor:
    # - Saves processor state (CPSR) to SPSR.
    # - Changes the current mode to supervisor mode (or another preferred mode).
    # mrs  : Move Register from Status register
    # spsr : Saved Program Status Register (mode, flags)
    mrs r1, spsr

    # call C interupt handler
    bl software_interupt_handler

    # restore user registers
    ldmfd sp!, {r0-r12, lr}

    subs pc, lr, #4

_irq_handler:
    # timers, etc.
    # pc = lr - #4 (lr = return adress)
    subs pc, lr, #4

// Unhandled interuptions
_undefined_handler:
_prefetch_abort:
_data_abort:
_reserved_handler:

_fiq_handler:
    // Default handler: hang
    b .
