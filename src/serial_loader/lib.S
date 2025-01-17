
.section ".text"

.globl loader_cpu_delay
loader_cpu_delay:
    subs r0, r0, #1
    bne loader_cpu_delay
    bx lr

.globl loader_mmio_write
loader_mmio_write:
    str r1,[r0]
    bx lr

.globl loader_mmio_read
loader_mmio_read:
    ldr r0,[r0]
    bx lr
