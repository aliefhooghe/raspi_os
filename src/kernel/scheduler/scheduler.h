#ifndef SATAN_SCHEDULER_H_
#define SATAN_SCHEDULER_H_

#include <stdint.h>

#include "task_context.h"

#define SCHEDULER_MAX_TASK_COUNT 0x80u


// allocations des stack pour le moment => une règle arithmétique en fonction du task_id id

// stack = 0xXXXXXXXXXXXX - task_id * TASK_STACK_SIZE

typedef struct {
    task_context_t context;
    int32_t id;
} task_t;

typedef struct {
    task_t tasks[SCHEDULER_MAX_TASK_COUNT];
    uint32_t current_task;
    uint32_t task_count;
    uint32_t id_gen;
    uint32_t stop_current_task;
} scheduler_t;


void scheduler_init(scheduler_t *scheduler);
void scheduler_start(scheduler_t *scheduler);

const task_context_t *scheduler_switch_task(
    scheduler_t *scheduler,
    const task_context_t *current_context
);

int32_t scheduler_add_task(
    scheduler_t *scheduler,
    uintptr_t proc_address,
    void* stack_address,
    uint32_t param
);

int32_t scheduler_get_current_task_id(
    scheduler_t *scheduler
);

void scheduler_stop_current_task(
    scheduler_t *scheduler
);

#endif
