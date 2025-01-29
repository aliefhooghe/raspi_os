#include <stdint.h>

#include "kernel.h"
#include "syscalls.h"

#include "hardware/cpu.h"
#include "hardware/mini_uart.h"
#include "hardware/watchdog.h"



typedef int32_t (*syscall_handler_t)(uint32_t arg0, uint32_t arg1);

/**
 *  System Call Handlers definition
 */
static int32_t _syscall__yield(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    return 42;
}

static int32_t _syscall__reboot(uint32_t arg0, uint32_t arg1)
{
    (void)arg0;
    (void)arg1;
    watchdog_init(0x100);
    return 0;
}

static int32_t _syscall__spawn(uint32_t proc_address, uint32_t stack_address)
{
    kernel_scheduler_add_task(proc_address, stack_address);
    return 0;
}

/**
 *  System Call Handler entrypoint
 */
static syscall_handler_t _syscall_table[SYSCALL_COUNT] =
{
    [SYSCALL_YIELD] = _syscall__yield,
    [SYSCALL_REBOOT] = _syscall__reboot,
    [SYSCALL_SPAWN] = _syscall__spawn
};

int32_t kernel_syscall_handler(
    syscall_num_t syscall_num,
    uint32_t arg0,
    uint32_t arg1)
{
    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_puts("[kernel] handling software interupt\r\n");
    mini_uart_printf("[kernel] cpu mode: 0x%x\r\n", cpu_mode);

    if (syscall_num >= SYSCALL_COUNT)
    {
        return -1;
    }
    else
    {
        syscall_handler_t handler = _syscall_table[syscall_num];
        return handler(arg0, arg1);
    }
}
