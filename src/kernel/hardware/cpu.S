
.section ".text"

.globl cpu_delay
cpu_delay:
    subs r0, r0, #1
    bne cpu_delay
    bx lr

.globl cpu_get_execution_mode
cpu_get_execution_mode:
    mrs r0, cpsr
    and r0, r0, #0x1f
    bx lr

.globl cpu_irq_enable
cpu_irq_enable:
    mrs r0, cpsr       // read Current Program Status Register
    bic r0, r0, #0x80  // clear bit 7: enable irq
    msr cpsr_c,r0      // write Current Program Status Register
    bx lr
