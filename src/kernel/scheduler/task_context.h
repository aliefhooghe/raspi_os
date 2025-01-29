#ifndef SATAN_SCHEDULER_TASK_CONTEXT_H_
#define SATAN_SCHEDULER_TASK_CONTEXT_H_

#include <stdint.h>

/**
 * task register which must be saved and restored when switching context
 */
typedef struct {
    uint32_t sp;    // stack pointer
    uint32_t spsr;  // saved status register

    uint32_t r1;    // user registers
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;

    uint32_t lr;    // return address register

    // Note: r0 is not saved as it is used to return the syscall status
} task_context_t;



#endif
