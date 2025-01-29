
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/mini_uart.h"

#include "lib/str.h"

#include "scheduler.h"
#include "task_context.h"


extern void __set_task_context(task_context_t *current_context);


void scheduler_init(scheduler_t *scheduler)
{
    _memset(scheduler, 0, sizeof(*scheduler));
}

void scheduler_start(scheduler_t *scheduler)
{
    if (scheduler->task_count > 0) {
        __set_task_context(&scheduler->task_contexts[0]);
    }
    else {
        mini_uart_puts("[kernel] FATAL ERROR: no task to be started\r\n");
        for (;;);
    }
}

task_id scheduler_add_task(
    scheduler_t *scheduler,
    uintptr_t proc_address,
    uintptr_t stack_address,
    uint32_t param)
{
    if (scheduler->task_count >= SCHEDULER_MAX_TASK_COUNT)
    {
        return TASK_ERROR;
    }

    // initialize the new task context
    const task_id new_task = scheduler->task_count++;
    task_context_t *new_context = &scheduler->task_contexts[new_task];

    _memset(new_context, 0, sizeof(task_context_t));
    new_context->r0 = param;
    new_context->sp = stack_address;
    new_context->lr = proc_address;
    new_context->spsr =
        CPU_CPSR_MODE_USER |
        CPU_CPSR_DISABLE_IRQ |
        CPU_CPSR_DISABLE_FIQ;

    return new_task;
}


const task_context_t *scheduler_switch_task(
    scheduler_t *scheduler,
    const task_context_t *current_context)
{
    mini_uart_printf("[kernel] save current task context:\r\n");

    mini_uart_printf("[kernel] spsr = 0x%x\r\n", current_context->spsr);
    mini_uart_printf("[kernel] sp   = 0x%x\r\n", current_context->sp);
    mini_uart_printf("[kernel] r0   = 0x%x\r\n", current_context->r0);
    mini_uart_printf("[kernel] r1   = 0x%x\r\n", current_context->r1);
    mini_uart_printf("[kernel] r2   = 0x%x\r\n", current_context->r2);
    mini_uart_printf("[kernel] r3   = 0x%x\r\n", current_context->r3);
    mini_uart_printf("[kernel] r4   = 0x%x\r\n", current_context->r4);
    mini_uart_printf("[kernel] r5   = 0x%x\r\n", current_context->r5);
    mini_uart_printf("[kernel] r6   = 0x%x\r\n", current_context->r6);
    mini_uart_printf("[kernel] r7   = 0x%x\r\n", current_context->r7);
    mini_uart_printf("[kernel] r8   = 0x%x\r\n", current_context->r8);
    mini_uart_printf("[kernel] r9   = 0x%x\r\n", current_context->r9);
    mini_uart_printf("[kernel] r10  = 0x%x\r\n", current_context->r10);
    mini_uart_printf("[kernel] r11  = 0x%x\r\n", current_context->r11);
    mini_uart_printf("[kernel] r12  = 0x%x\r\n", current_context->r12);
    mini_uart_printf("[kernel] lr   = 0x%x\r\n", current_context->lr);

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
