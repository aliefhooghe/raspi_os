#ifndef SATAN_SCHEDULER_TASK_CONTEXT_H_
#define SATAN_SCHEDULER_TASK_CONTEXT_H_

#include <stdint.h>

// 
// Cpu registers which must be saved and restored when switching context
//
// Layout is important. See interupt.S / scheduler.S
typedef struct {
    uint32_t r0;      // syscall return status

    uint32_t lr_usr;  // user return address register
    uint32_t sp;      // stack pointer
    uint32_t spsr;    // saved status register

    uint32_t r1;      // user registers
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

    uint32_t lr_svc;    // svc return address register: pc_usr + 4
} task_context_t;

// if this change fix is needed in svc_handler (interupt.S)
_Static_assert(
    sizeof(task_context_t) == 68u,
    "sizeof(task_context_t) as changed"
);

#endif
