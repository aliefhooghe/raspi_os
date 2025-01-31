#ifndef SATAN_KERNEL_H_
#define SATAN_KERNEL_H_

#include <stdint.h>

#include "scheduler/scheduler.h"

typedef struct {
    scheduler_t scheduler;
} kernel_state_t;

// kernel entry point. Called from reset handler
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags);

// kernel state interface: called from svc handler
const task_context_t *kernel_switch_task(const task_context_t *current_context);

#endif
