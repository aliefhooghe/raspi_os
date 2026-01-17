#include <stdint.h>

#include "usr_syscalls.h"
#include "syscalls.h"

// perform a syscall
extern int32_t __syscall(uint32_t syscall_num, ...);

static inline int32_t syscall(uint32_t syscall_num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t status = __syscall(syscall_num, arg0, arg1, arg2);
    asm volatile ("" ::: "memory"); // memory barrier
    return status;
}

// -- SYSTEM

int32_t usr_syscall_yield(int32_t status)
{
    return syscall(SYSCALL__YIELD, (uint32_t)status, 0u, 0u);
}

int32_t usr_syscall_reboot(void)
{
    return syscall(SYSCALL__REBOOT, 0u, 0u, 0u);
}

int32_t usr_syscall_exit(int32_t status)
{
    syscall(SYSCALL__EXIT, status, 0u, 0u);
    // should not be reached
    return 0;
}

// --  PROCESSES

int32_t usr_syscall_fork(void)
{
    return syscall(SYSCALL__FORK, 0u, 0u, 0u);
}
int32_t usr_syscall_getpid(void)
{
    return syscall(SYSCALL__GETPID, 0u, 0u, 0u);
}

int32_t usr_syscall_getppid(void)
{
    return syscall(SYSCALL__GETPPID, 0u, 0u, 0u);
}

int32_t usr_syscall_waitpid(int32_t pid, int32_t *wstatus)
{
    return syscall(SYSCALL__WAITPID, pid, (uint32_t)wstatus, 0u);
}

int32_t usr_syscall_exec(const char *bin)
{
    return syscall(SYSCALL__EXEC, (uint32_t)bin, 0u, 0u);
}

// --  Filesystem

int32_t usr_syscall_open(const char *path, int32_t flags, int32_t mode)
{
    return syscall(SYSCALL__OPEN, (uint32_t)path, flags, mode);
}

int32_t usr_syscall_close(int32_t fd)
{
    return syscall(SYSCALL__CLOSE, (uint32_t)fd, 0u, 0u);
}

size_t usr_syscall_read(int32_t fd, void *data, size_t size)
{
    return syscall(SYSCALL__READ, fd, (uint32_t)data, size);
}

size_t usr_syscall_readdir(int32_t fd, dirent *entries, size_t count)
{
    return syscall(SYSCALL__READDIR, fd, (uint32_t)entries, count);
}

size_t usr_syscall_write(int32_t fd, const void *data, size_t size)
{
    return syscall(SYSCALL__WRITE, fd, (uint32_t)data, size);
}

off_t usr_syscall_lseek(int fd, off_t offset, int whence)
{
    return syscall(SYSCALL__LSEEK, fd, (uint32_t)offset, whence);
}

int32_t usr_syscall_mount(const char *dev, const char *target, const char *fstype)
{
    return syscall(SYSCALL__MOUNT, (uint32_t)dev, (uint32_t)target, (uint32_t)fstype);
}

int32_t usr_syscall_mkdir(const char *path, mode_t mode)
{
    return syscall(SYSCALL__MKDIR, (uint32_t)path, mode, 0u);
}

int32_t usr_syscall_mknod(const char *path, mode_t mode, dev_t dev)
{
    return syscall(SYSCALL__MKNOD, (uint32_t)path, mode, dev);
}
