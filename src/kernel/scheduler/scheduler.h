#ifndef SATAN_SCHEDULER_H_
#define SATAN_SCHEDULER_H_

#include <stdint.h>

#include "task_context.h"

#define SCHEDULER_MAX_TASK_COUNT 0x6u

typedef int32_t task_id;
#define TASK_ERROR ((task_id)-1)


// allocations des stack pour le moment => une règle arithmétique en fonction du task_id id

// stack = 0xXXXXXXXXXXXX - task_id * TASK_STACK_SIZE

typedef struct {
    task_context_t task_contexts[SCHEDULER_MAX_TASK_COUNT];
    uint32_t current_task;
    uint32_t task_count;
} scheduler_t;

void scheduler_init(scheduler_t *scheduler);
void scheduler_start(scheduler_t *scheduler);

task_id scheduler_add_task(
    scheduler_t *scheduler,
    uintptr_t proc_address,
    uintptr_t stack_address,
    uint32_t param);

const task_context_t *scheduler_switch_task(
    scheduler_t *scheduler,
    const task_context_t *current_context);

#endif
