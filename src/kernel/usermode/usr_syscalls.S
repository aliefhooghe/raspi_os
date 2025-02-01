
    .section ".text"

    .globl syscall
syscall:
    //  syscall number     : r0
    //  syscall arguments  : r1 = arg0, r2 = arg1, r3 = arg2
    svc #0
    bx lr
