
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
