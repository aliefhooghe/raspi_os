
    .section ".data"
msg:
    .asciz "Unexpected return to scheduler start avec initial SVC"


    .section ".text"

    .globl __scheduler_set_task_context
__scheduler_set_task_context:
    // r0: task_context_t* task_context
    // r1: uint32_t *translation_table

    // restore the task context

    // 1 - push the syscall status and translation table
    ldr r2, [r0]            // load saved r0 (syscall return status) from context
    push {r2}               // push status on the stack
    add r0, r0, #4

    push {r1}               // push translation table address

    // 2 - restore the **user mode** stack pointer and lr
    ldr r1, [r0]            // load saved lr_usr from context
    add r0, r0, #4
    
    ldr r2, [r0]            // load saved sp from context
    add r0, r0, #4

    // TODO: are we not re enabling exception here by accident ?????
    mrs r3, cpsr
    orr r3, #0x1F           // switch to the system cpu mode (0x1F)
    msr cpsr_c, r3

    mov lr, r1              // restore lr
    mov sp, r2              // restore sp

    mrs r2, cpsr
    bic r2, r2, #0x1F
    orr r2, #0x13           // switch to the SVC cpu mode (0x13)
    msr cpsr_c, r2

    // 3 - restore the saved program status register
    ldr r1, [r0]            // load saved spsr
    add r0, r0, #4
    msr spsr, r1            // restore spsr

    // 4 - restore user register and lr_svc
    ldmfd r0, {r1-r12, lr}

    // 5 - switch to process memory space
    pop {r0}                // r0 = translation_table
    push {lr}
    bl mmu_set_translation_table
    pop {lr}

    // 6 - restore syscall return status into r0
    pop {r0}

    // V - return to the next scheduled task
    // cpu status will be restored from spsr
    movs pc, lr

    .globl __scheduler_exec_init
__scheduler_exec_init:
    //  args: r0 = path virtual address

    //  syscall: exec
    //  syscall number     : r0 = EXEC = 14
    //  syscall arguments  : r1 = path, r2 = argv = NULL, r3 = NULL
    mov r1, r0
    mov r0, #14
    mov r2, #0
    mov r3, #0
    svc #0

    // we should never reach here.
    ldr r0, =msg
    b kernel_fatal_error

