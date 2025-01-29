#ifndef SATAN_KERNEL_H_
#define SATAN_KERNEL_H_

#include "scheduler/scheduler.h"

// kernel entry point. Called from reset handler
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags);

// kernel state interface
const task_context_t *kernel_switch_task(const task_context_t *current_context);
task_id kernel_scheduler_add_task(uintptr_t proc_address, uintptr_t stack_address);

#endif
