
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "hardware/cpu.h"

#include "lib/str.h"

#include "scheduler.h"
#include "task_context.h"
#include "vfs/vfs.h"


extern void __set_task_context(task_context_t *current_context);


void scheduler_init(scheduler_t *scheduler)
{
    _memset(scheduler, 0, sizeof(*scheduler));
}

void scheduler_start(scheduler_t *scheduler)
{
    if (scheduler->task_count > 0) {
        __set_task_context(&scheduler->tasks[0].context);
    }
    else {
        kernel_fatal_error("no task to be started");
    }
}

const task_context_t *scheduler_switch_task(
    scheduler_t *scheduler,
    const task_context_t *current_context)
{
    if (scheduler->stop_current_task)
    {
        scheduler->stop_current_task = 0u;

        // remove the current task. No state to save
        _memmove(
            &scheduler->tasks[scheduler->current_task],
            &scheduler->tasks[scheduler->current_task + 1],
            sizeof(task_t) * (scheduler->task_count - (scheduler->current_task + 1)));
        scheduler->task_count--;

        // index stay the same
    }
    else {
        // save the current task context context
        _memcpy(
            &scheduler->tasks[scheduler->current_task].context,
            current_context,
            sizeof(task_context_t));

        // compute the next task index
        scheduler->current_task = scheduler->current_task + 1;
    }

    // round robin
    if (scheduler->task_count == 0u)
    {
        kernel_fatal_error("the last running task was stopped");
    }
    if (scheduler->current_task == scheduler->task_count)
    {
        scheduler->current_task = 0u;
    }

    // switch to next task
    return &scheduler->tasks[scheduler->current_task].context;
}

static void scheduler_task_init(
    vfs_t *vfs,
    task_t *new_task, uint32_t task_id,
    void *stack_address, uintptr_t proc_address,
    uint32_t param)
{
    _memset(new_task, 0, sizeof(task_t));

    // set pid
    new_task->id = task_id;

    // set initial task context
    new_task->context.r0 = param;
    new_task->context.sp = (uint32_t)stack_address;
    new_task->context.lr = proc_address;
    new_task->context.spsr =
        CPU_CPSR_MODE_USER |
        CPU_CPSR_DISABLE_IRQ |
        CPU_CPSR_DISABLE_FIQ;

    // setup stdin/stdout
    const file_descriptor_t tty_fd = vfs_get_tty_file_descriptor(vfs);
    new_task->fd_count = 2u;
    new_task->file_descriptors[0] = tty_fd;
    new_task->file_descriptors[1] = tty_fd;
}

int32_t scheduler_add_task(
    scheduler_t *scheduler,
    vfs_t *vfs,
    uintptr_t proc_address,
    void* stack_address,
    uint32_t param)
{

    if (scheduler->task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return -1;
    }

    // initialize the new task context
    const int32_t new_task_id = scheduler->id_gen++;
    const uint32_t new_index = scheduler->task_count++;

    task_t *new_task = &scheduler->tasks[new_index];
    scheduler_task_init(
        vfs,
        new_task, new_task_id,
        stack_address, proc_address, param);

    return new_task_id;
}

void scheduler_cur_proc_exit(scheduler_t *scheduler)
{
    scheduler->stop_current_task = 1u;
}


int32_t scheduler_cur_proc_get_id(scheduler_t *scheduler)
{
    if (scheduler->current_task < scheduler->task_count)
    {
        return scheduler->tasks[scheduler->current_task].id;
    }
    else
    {
        return -1;
    }
}

file_descriptor_t *scheduler_cur_proc_get_fd(
    scheduler_t *scheduler,
    int32_t fd)
{
    task_t *current_task = &scheduler->tasks[scheduler->current_task];
    if ((uint32_t)fd >= current_task->fd_count)
        return NULL;
    return &current_task->file_descriptors[fd];
}
