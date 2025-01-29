
    .section ".text"

    .globl __set_task_context
__set_task_context:
    // r0: task_context_t* task_context

    // restore the task context

    // 1 - push the syscall status
    ldr r1, [r0]            // load saved r0 (syscall return status) from context
    push {r1}               // push status on the stack
    add r0, r0, #4

    // IV.2 - restore the **user mode** stack pointer
    ldr r1, [r0]            // load saved sp from context
    add r0, r0, #4

    mov r2, #0x1F           // switch to the system cpu mode (0x1F)
    msr cpsr_c, r2

    mov sp, r1              // restore sp

    mov r2, #0x13           // switch to the SVC cpu mode (0x13)
    msr cpsr_c, r2

    // IV.3 - restore the saved program status register
    ldr r1, [r0]            // load saved spsr
    add r0, r0, #4
    msr spsr, r1            // restore spsr

    // IV.4 - restore user registers on the stack
    ldmfd r0, {r1-r12, lr}

    // IV.5 - restore syscall return status into r0
    pop {r0}

    // V - return to the next scheduled task
    // cpu status will be restored from spsr
    movs pc, lr
