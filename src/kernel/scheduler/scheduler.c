
#include <stdint.h>

#include "lib/str.h"

#include "scheduler.h"
#include "task_context.h"



void scheduler_init(scheduler_t *scheduler)
{
    _memset(scheduler, 0, sizeof(*scheduler));
    scheduler->task_count = 1u; // baaaaad
}


task_id scheduler_add_task(
    scheduler_t *scheduler,
    uintptr_t proc_address,
    uintptr_t stack_address)
{
    if (scheduler->task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return TASK_ERROR;
    }

    // clear the new task context.
    const task_id new_task = scheduler->task_count++;
    _memset(&scheduler->task_contexts[new_task], 0, sizeof(task_context_t));

    // set the return address to the function start
    scheduler->task_contexts[new_task].lr = proc_address;
    // how to set and pass the stack ?

    return new_task;
}


const task_context_t *scheduler_switch_task(
    scheduler_t *scheduler,
    const task_context_t *current_context)
{
    // save the current task context context
    _memcpy(
        &scheduler->task_contexts[scheduler->current_task],
        current_context,
        sizeof(task_context_t));

    // switch to next task (avoid modulo)
    scheduler->current_task = (scheduler->current_task + 1u);
    if (scheduler->current_task == scheduler->task_count) {
        scheduler->current_task = 0u;
    }
    return &scheduler->task_contexts[scheduler->current_task];
}
