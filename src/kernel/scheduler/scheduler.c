
#include <stdint.h>

#include "kernel.h"
#include "hardware/cpu.h"
#include "hardware/mini_uart.h"

#include "lib/str.h"

#include "scheduler.h"
#include "task_context.h"


extern void __set_task_context(task_context_t *current_context);

static int32_t _scheduler_task_index_by_pid(
    scheduler_t *scheduler,
    int32_t pid)
{
    for (uint32_t index = 0u; index < scheduler->task_count; index++) {
        if (scheduler->tasks[index].id == pid)
            return index;
    }

    return -1;
}

static void mini_uart_print_context(const task_context_t *context)
{
    mini_uart_printf("[kernel] spsr = 0x%x\r\n", context->spsr);
    mini_uart_printf("[kernel] sp   = 0x%x\r\n", context->sp);
    mini_uart_printf("[kernel] r0   = 0x%x\r\n", context->r0);
    mini_uart_printf("[kernel] r1   = 0x%x\r\n", context->r1);
    mini_uart_printf("[kernel] r2   = 0x%x\r\n", context->r2);
    mini_uart_printf("[kernel] r3   = 0x%x\r\n", context->r3);
    mini_uart_printf("[kernel] r4   = 0x%x\r\n", context->r4);
    mini_uart_printf("[kernel] r5   = 0x%x\r\n", context->r5);
    mini_uart_printf("[kernel] r6   = 0x%x\r\n", context->r6);
    mini_uart_printf("[kernel] r7   = 0x%x\r\n", context->r7);
    mini_uart_printf("[kernel] r8   = 0x%x\r\n", context->r8);
    mini_uart_printf("[kernel] r9   = 0x%x\r\n", context->r9);
    mini_uart_printf("[kernel] r10  = 0x%x\r\n", context->r10);
    mini_uart_printf("[kernel] r11  = 0x%x\r\n", context->r11);
    mini_uart_printf("[kernel] r12  = 0x%x\r\n", context->r12);
    mini_uart_printf("[kernel] lr   = 0x%x\r\n", context->lr);

}

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
    const int32_t current_id = scheduler_get_current_task_id(scheduler);

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

    const int32_t next_id = scheduler_get_current_task_id(scheduler);
    mini_uart_printf(
        "[kernel] kernel switch task %u -> %u\r\n",
        current_id, next_id);

    // switch to next task
    return &scheduler->tasks[scheduler->current_task].context;
}

int32_t scheduler_add_task(
    scheduler_t *scheduler,
    uintptr_t proc_address,
    uintptr_t stack_address,
    uint32_t param)
{

    if (scheduler->task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return -1;
    }

    // initialize the new task context
    const int32_t new_task_id = scheduler->id_gen++;
    const uint32_t new_index = scheduler->task_count++;

    mini_uart_printf("[kernel] add task with pid %u (index=%u)\r\n", new_task_id, new_index);

    task_t *new_task = &scheduler->tasks[new_index];

    _memset(new_task, 0, sizeof(task_t));
    new_task->id = new_task_id;
    new_task->context.r0 = param;
    new_task->context.sp = stack_address;
    new_task->context.lr = proc_address;
    new_task->context.spsr =
        CPU_CPSR_MODE_USER |
        CPU_CPSR_DISABLE_IRQ |
        CPU_CPSR_DISABLE_FIQ;

    return new_task_id;
}


int32_t scheduler_get_current_task_id(scheduler_t *scheduler)
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


void scheduler_stop_current_task(scheduler_t *scheduler)
{
    const int32_t current_id = scheduler_get_current_task_id(scheduler);
    mini_uart_printf("[kernel] stop currrent task: pid=%u\r\n", current_id);
    scheduler->stop_current_task = 1u;
}
