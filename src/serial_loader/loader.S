

.section ".text"


.globl __mmio_write
__mmio_write:
    str r1,[r0]
    bx lr

.globl __mmio_read
__mmio_read:
    ldr r0,[r0]
    bx lr

.globl __cpu_delay
__cpu_delay:
    subs r0, r0, #1
    bne __cpu_delay
    bx lr
