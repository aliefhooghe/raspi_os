
.section ".text"

.globl cpu_delay
cpu_delay:
    subs r0, r0, #1
    bne cpu_delay
    bx lr
