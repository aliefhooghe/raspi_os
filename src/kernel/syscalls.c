
#include "syscalls.h"
#include "hardware/watchdog.h"

#include <stdint.h>

typedef int32_t (*syscall_handler_t)(uint32_t arg0, uint32_t arg1);


/**
 *  System Call Handlers definition
 */
static int32_t _syscall__dummy(uint32_t arg0, uint32_t arg1)
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


/**
 *  System Call Handler entrypoint
 */
static syscall_handler_t _syscall_table[SYSCALL_COUNT] =
{
    [SYSCALL_DUMMY] = _syscall__dummy,
    [SYSCALL_REBOOT] = _syscall__reboot
};

int32_t kernel_syscall_handler(
    syscall_num_t syscall_num,
    uint32_t arg0, uint32_t arg1)
{
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
