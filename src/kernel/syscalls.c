#include <stdint.h>

#include "kernel.h"
#include "syscalls.h"

#include "hardware/cpu.h"
#include "hardware/mini_uart.h"
#include "hardware/watchdog.h"

#include "scheduler/scheduler.h"

/**
 *  syscall handler function pointer type
 */
typedef int32_t (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2);


/**
 *  global kernel state is accessed by the syscall handlers
 */
extern kernel_state_t __kernel_state;


/**
 *  System Call Handlers definition
 */
static int32_t _syscall__yield(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return 0;
}

static int32_t _syscall__reboot(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    watchdog_init(0x100);
    return 0;
}

static int32_t _syscall__spawn(uint32_t proc_address, uint32_t stack_address, uint32_t param)
{
    if (TASK_ERROR == scheduler_add_task(&__kernel_state.scheduler, proc_address, stack_address, param))
    {
        return SYSCALL_STATUS_ERR;
    }
    else
    {
        return SYSCALL_STATUS_OK;
    }
}

static int32_t _syscall__exit(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return SYSCALL_STATUS_ERR;
}

/**
 *  System Call Handler entrypoint
 */
static syscall_handler_t _syscall_table[SYSCALL_COUNT] =
{
    [SYSCALL_YIELD] = _syscall__yield,
    [SYSCALL_REBOOT] = _syscall__reboot,
    [SYSCALL_SPAWN] = _syscall__spawn,

    [SYSCALL_EXIT] = _syscall__exit,
    // [SYSCALL_READ] = _syscall__read,
    // [SYSCALL_WRITE] = _syscall__write,
    // [SYSCALL_OPEN] = _syscall__open,
    // [SYSCALL_CLOSE] = _syscall__close,
};

int32_t kernel_syscall_handler(
    syscall_num_t syscall_num,
    uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_puts("[kernel] handling software interupt\r\n");
    mini_uart_printf("[kernel] cpu mode: 0x%x\r\n", cpu_mode);

    // ici typiquement, il faudrait le kernel state.
    // voire: que ici ??  a part l'init on dirait bien.

    if (syscall_num >= SYSCALL_COUNT)
    {
        return -1;
    }
    else
    {
        syscall_handler_t handler = _syscall_table[syscall_num];
        return handler(arg0, arg1, arg2);
    }
}
