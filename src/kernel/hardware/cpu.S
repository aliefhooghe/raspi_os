
.section ".text"

    .globl cpu_delay
cpu_delay:
    subs r0, r0, #1
    nop
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
    msr cpsr_c, r0     // write Program Status Register Control
    bx lr

    .globl cpu_irq_disable
cpu_irq_disable:
    mrs r0, cpsr       // read Current Program Status Register
    orr r0, r0, #0x80  // set bit 7: disable irq
    msr cpsr_c, r0     // write Program Status Register Control
    bx lr

    .globl cpu_fiq_enable
cpu_fiq_enable:
    mrs r0, cpsr       // read Current Program Status Register
    bic r0, r0, #0x40  // clear bit 6: enable fiq
    msr cpsr_c, r0     // write Program Status Register Control
    bx lr

    .globl cpu_fiq_disable
cpu_fiq_disable:
    mrs r0, cpsr       // read Current Program Status Register
    orr r0, r0, #0x40  // set bit 6: disable fiq
    msr cpsr_c, r0     // write Program Status Register Control
    bx lr
    
    .globl cpu_wait_for_interupt
cpu_wait_for_interupt:
    wfi
    bx lr

