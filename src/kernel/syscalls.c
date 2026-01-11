
#include <stddef.h>
#include <stdint.h>

#include "elf_loader/elf_loader.h"
#include "hardware/mini_uart.h"
#include "hardware/watchdog.h"
#include "kernel.h"
#include "kernel_types.h"
#include "scheduler/scheduler.h"
#include "syscalls.h"
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
    const int32_t status = arg0;
    (void)arg1;
    (void)arg2;
    return status;
}

static int32_t _syscall__REBOOT(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    watchdog_reboot();
    return 0;
}

static int32_t _syscall__EXIT(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg1;
    (void)arg2;
    const int32_t status = arg0;
    scheduler_cur_proc_exit(status);
    return SYSCALL_STATUS_OK;
}

static int32_t _syscall__OPEN(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const char *path = (const char*)scheduler_cur_proc_get_kernel_address(arg0);
    const uint32_t flags = arg1;
    const uint32_t mode = arg2;

    file_t *file = vfs_file_open(path, flags, mode);
    if (file == NULL)
        return SYSCALL_STATUS_ERR;

    // we shoule be able to allocate the fd before ? 
    const int fd = scheduler_cur_proc_add_fd(file);
    if (fd < 0)
        vfs_file_close(file);

    return fd;
}

static int32_t _syscall__CLOSE(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    (void)arg1;
    (void)arg2;

    file_t *file = scheduler_cur_proc_get_fd(fd);
    if (file == NULL)
        return SYSCALL_STATUS_ERR;

    scheduler_cur_proc_rem_fd(fd);   // decrement file.fcount
    vfs_file_close(file);            // decrement inode.icount

    return 0;
}

static int32_t _syscall__READ(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    void* data = scheduler_cur_proc_get_kernel_address(arg1);
    const size_t size = arg2;
  
    file_t *file = scheduler_cur_proc_get_fd(fd);
    if (file == NULL)
        return SYSCALL_STATUS_ERR;

    return vfs_file_read(file, data, size);
}

static int32_t _syscall__READDIR(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    dirent* entries = (dirent*)scheduler_cur_proc_get_kernel_address(arg1);
    const size_t count = arg2;
  
    file_t *file = scheduler_cur_proc_get_fd(fd);
    if (file == NULL)
        return SYSCALL_STATUS_ERR;

    return vfs_file_readdir(file, entries, count);
}

static int32_t _syscall__WRITE(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    const void* data = scheduler_cur_proc_get_kernel_address(arg1);
    const size_t size = arg2;
    mini_uart_kernel_log("write from paddr %x", data);
    file_t *file = scheduler_cur_proc_get_fd(fd);
    if (file == NULL)
        return SYSCALL_STATUS_ERR;

    return vfs_file_write(file, data, size);
}

static int32_t _syscall__MOUNT(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const char *dev = (const char*)scheduler_cur_proc_get_kernel_address(arg0);
    const char *target = (const char*)scheduler_cur_proc_get_kernel_address(arg1);
    const char *fstype = (const char*)scheduler_cur_proc_get_kernel_address(arg2);
    (void)dev;
    (void)target;
    (void)fstype;
    kernel_fatal_error("mount syscall is not implemented");
    return -1;
}

static int32_t _syscall__MKDIR(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg2;
    const char *path = (const char*)scheduler_cur_proc_get_kernel_address(arg0);
    const mode_t mode = arg1;
    return vfs_mkdir(path, mode);
}

static int32_t _syscall__MKNOD(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const char *path = (const char*)scheduler_cur_proc_get_kernel_address(arg0);
    const mode_t mode = arg1;
    const dev_t device = arg2;
    return vfs_mknod(path, mode, device);
}

static int32_t _syscall__LSEEK(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t fd = arg0;
    const off_t offset = arg1;
    const int whence = arg2;

    file_t *file = scheduler_cur_proc_get_fd(fd);
    if (file == NULL)
        return SYSCALL_STATUS_ERR;

    return vfs_file_lseek(file, offset, whence);
}

static int32_t _syscall__FORK(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;

    // will return the child pid to the parent process
    return scheduler_cur_proc_fork();
}

static int32_t _syscall__WAITPID(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg2;
    const int32_t pid = arg0;
    uint32_t *const wstatus = (uint32_t*)scheduler_cur_proc_get_kernel_address(arg1);
    return scheduler_cur_proc_wait_id(pid, wstatus);
}

static int32_t _syscall__EXEC(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    (void)arg1;
    (void)arg2;
    const char *path = (const char*)scheduler_cur_proc_get_kernel_address(arg0);
    const int status = scheduler_cur_proc_exec(path);
    return status;
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
        mini_uart_kernel_log("syscall: invalid number: %u", syscall_num);
        scheduler_cur_proc_set_syscall_status(-1);
    }
    else
    {
        // find calling process
        const int32_t calling_pid = scheduler_cur_proc_get_id();
        mini_uart_kernel_log(
            "syscall: %s pid=%u args=0x%x 0x%x 0x%x",
            _syscall_names[syscall_num], calling_pid,
            arg0, arg1, arg2);

        // call the relevant syscall handler
        syscall_handler_t handler = _syscall_table[syscall_num];
        const int32_t status = handler(arg0, arg1, arg2);
        mini_uart_kernel_log("syscall: status=0x%x", status);

        // if the calling process was removed: do not write status
        if (calling_pid == scheduler_cur_proc_get_id())
        {
            // if the process was not descheduled
            scheduler_cur_proc_set_syscall_status(status);
        }
    }
}
