
#include <stddef.h>
#include <stdint.h>

#include "scheduler/scheduler.h"
#include "syscalls.h"

#include "hardware/watchdog.h"

#include "memory/allocator.h"

#include "vfs/vfs.h"

#define DEFAULT_TASK_SIZE 0x1000u // 4 KB

/**
 *  syscall handler function pointer type
 */
typedef int32_t (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2);


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

static int32_t _syscall__spawn(uint32_t proc_address, uint32_t param, uint32_t arg2)
{
    (void)arg2;
    void *task_stack = mem_alloc(DEFAULT_TASK_SIZE);
    const int32_t pid = scheduler_add_task(proc_address, task_stack, param);

    // bug here
    if (pid < 0)
    {
        return SYSCALL_STATUS_ERR;
    }
    else
    {
        return pid;
    }
}

static int32_t _syscall__exit(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg1;
    (void)arg2;
    const int32_t status = arg0;
    (void)status; // TODO: handle the status.
    scheduler_cur_proc_exit();
    return SYSCALL_STATUS_OK;
}

static int32_t _syscall__read(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    void* data = (void*)arg1;
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

static int32_t _syscall__write(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    const void* data = (const void*)arg1;
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

static int32_t _syscall__getpid(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    return scheduler_cur_proc_get_id();
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
    [SYSCALL_READ] = _syscall__read,
    [SYSCALL_WRITE] = _syscall__write,
    // [SYSCALL_OPEN] = _syscall__open,
    // [SYSCALL_CLOSE] = _syscall__close,

    [SYSCALL_GETPID] = _syscall__getpid
};

int32_t kernel_syscall_handler(
    syscall_num_t syscall_num,
    uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
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
