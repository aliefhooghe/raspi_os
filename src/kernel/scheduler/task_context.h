#ifndef SATAN_SCHEDULER_TASK_CONTEXT_H_
#define SATAN_SCHEDULER_TASK_CONTEXT_H_

#include <stdint.h>

typedef struct {

    // task register which must be saved and restored when switching context
    // r0 is not saved as it is used to return the syscall status
    uint32_t r1;
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
    uint32_t lr;
} task_context_t;



#endif
