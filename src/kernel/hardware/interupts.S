


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
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9} // load r2-r9 from *r0++
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9} // store r2-r9 to *r1++ (8 registers * 4 byte = 32 bytes)
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9} // again, 32 more bytes => 64 bytes
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}

    /**
     *  Setup the stacks for the different cpu modes
     *
     *    - irq mode: sp = 0x00008000
     *    - fiq mode: sp = 0x00004000
     *    - sv  mode: sp = 0x08000000
     */

    // switch to mode IRQ
    // PSR_IRQ_MODE = 0b10010 (0x12), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0xD2 = (0x80 | 0x40 | 0x12)
    mov r0, #0xD2
    msr cpsr_c,r0
    mov sp, #0x8000

    // switch to mode FIQ (Fast Interrupt Mode)
    // PSR_FIQ_MODE = 0b10001 (0x11), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0xD1 = (0x80 | 0x40 | 0x11)
    mov r0, #0xD1
    msr cpsr_c, r0
    mov sp, #0x4000

    // switch to supervisor mode
    // PSR_SVC_MODE = 0b10011 (0x13), PSR_FIQ_DIS = 0x40, PSR_IRQ_DIS = 0x80
    // 0xD3 = (0x80 | 0x40 | 0x13)
    mov r0, 0xD3
    msr cpsr_c, r0
    mov sp, #0x8000000


    bl kernel_main
hang: b hang

@ 	// Clear out bss.
@ 	ldr r4, =__bss_start
@ 	ldr r9, =__bss_end
@ 	mov r5, #0
@ 	mov r6, #0
@ 	mov r7, #0
@ 	mov r8, #0
@ 	b       2f

@ 1:
@ 	// store multiple at r4.
@ 	stmia r4!, {r5-r8}

@ 	// If we are still below bss_end, loop.
@ 2:
@ 	cmp r4, r9
@ 	blo 1b



@ 	// Call kernel_main
@ 	ldr r3, =kernel_main
@ 	blx r3

@ 	// halt
@ halt:
@ 	wfe
@ 	b halt

_svc_handler:
    #   syscall number      : from lr
    #   syscall arguments   : r0, r1
    #

    # save user registers on the stack
    @ stmfd sp!, {r1-r12, lr}

    @ mov r2, r0 // get 1st argument to r2
    @ mov r3, r1 // get 2nd argument to r3

    @ // Get syscall number in r0
    @ ldr r0, [lr, #-4]
    @ and r0, r0, #0xFF

    @ // Get program status in r1
    @ // When an exception (like SVC) occurs, the processor:
    @ // - Saves processor state (CPSR) to SPSR.
    @ // - Changes the current mode to supervisor mode (or another preferred mode).
    @ // mrs  : Move Register from Status register
    @ // spsr : Saved Program Status Register (mode, flags)
    @ mrs r1, spsr

    @ // call C interupt handler
    @ bl software_interupt_handler

    @ // restore user registers
    @ ldmfd sp!, {r1-r12, lr}

    subs pc, lr, #4

_irq_handler:
    // timers, etc.
    // pc = lr - #4 (lr = return adress)
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

// TODO: to be tested.
.globl syscall
syscall:
    svc #1
    bx lr
