
#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"

#include "hardware/mini_uart.h"
#include "scheduler/scheduler.h"
#include "hardware/watchdog.h"

#include "vfs/vfs.h"

/**
 *  syscall handler function pointer type
 */
typedef int32_t (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2);

/**
 *  syscall names for debug
 */
#define GENERATE_STRING(ENUM) #ENUM,
static const char *_syscall_names[SYSCALL_COUNT] = {
    FOREACH_SYSCALL(GENERATE_STRING)
};

/**
 *  System Call Handlers definition
 */
static int32_t _syscall__YIELD(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return 0;
}

static int32_t _syscall__REBOOT(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    watchdog_init(0x100);
    return 0;
}

static int32_t _syscall__EXIT(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg1;
    (void)arg2;
    const int32_t status = arg0;
    (void)status; // TODO: handle the status.
    scheduler_cur_proc_exit();
    return SYSCALL_STATUS_OK;
}

static int32_t _syscall__READ(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    void* data = scheduler_cur_proc_get_kernel_address(arg1);
    const size_t size = arg2;
  
    file_descriptor_t *descriptor = scheduler_cur_proc_get_fd(fd);
    if (descriptor == NULL)
    {
        return SYSCALL_STATUS_ERR;
    }
    else
    {
        return file_descriptor_read(descriptor, data, size);
    }
}

static int32_t _syscall__WRITE(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    const void* data = scheduler_cur_proc_get_kernel_address(arg1);
    const size_t size = arg2;

    file_descriptor_t *descriptor = scheduler_cur_proc_get_fd(fd);

    if (descriptor == NULL)
    {
        return SYSCALL_STATUS_ERR;
    }
    else
    {
        return file_descriptor_write(descriptor, data, size);
    }
}

static int32_t _syscall__FORK(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;

    // will return the child pid to the parent process
    return scheduler_cur_proc_fork();
}

static int32_t _syscall__GETPPID(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return scheduler_cur_proc_get_parent_id();
}

static int32_t _syscall__GETPID(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return scheduler_cur_proc_get_id();
}

/**
 *  System Call Handler entrypoint
 */
#define GENERATE_HANDLER(ENUM) [SYSCALL__##ENUM] = _syscall__##ENUM,
static syscall_handler_t _syscall_table[SYSCALL_COUNT] =
{
    FOREACH_SYSCALL(GENERATE_HANDLER)
};

void kernel_syscall_handler(
    syscall_num_t syscall_num,
    uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    if (syscall_num >= SYSCALL_COUNT)
    {
        scheduler_cur_proc_set_syscall_status(-1);
    }
    else
    {
        const uint32_t pid = scheduler_cur_proc_get_id();
        mini_uart_kernel_log(
            "handle syscall %s for process pid=%u",
            _syscall_names[syscall_num], pid);
        syscall_handler_t handler = _syscall_table[syscall_num];
        const int32_t status = handler(arg0, arg1, arg2);
        mini_uart_kernel_log("syscall return status=%u", status);
        scheduler_cur_proc_set_syscall_status(status);
    }
}
