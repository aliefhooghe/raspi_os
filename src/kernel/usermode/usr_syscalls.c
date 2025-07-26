#include <stdint.h>

#include "usermode/usr_syscalls.h"
#include "syscalls.h"

// perform a syscall
extern int32_t __syscall(uint32_t syscall_num, ...);

static inline int32_t syscall(uint32_t syscall_num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t status = __syscall(syscall_num, arg0, arg1, arg2);
    asm volatile ("" ::: "memory"); // memory barrier
    return status;
}

int32_t usr_syscall_yield(void)
{
    return syscall(SYSCALL__YIELD, 0, 0, 0);
}

int32_t usr_syscall_reboot(void)
{
    return syscall(SYSCALL__REBOOT, 0, 0, 0);
}

int32_t usr_syscall_exit(int32_t status)
{
    return syscall(SYSCALL__EXIT, status, 0, 0);
}

size_t usr_syscall_read(int32_t fd, void *data, size_t size)
{
    return syscall(SYSCALL__READ, fd, (uint32_t)data, size);
}

size_t usr_syscall_write(int32_t fd, const void *data, size_t size)
{
    return syscall(SYSCALL__WRITE, fd, (uint32_t)data, size);
}

int32_t usr_syscall_fork(void)
{
    return syscall(SYSCALL__FORK, 0u, 0u, 0u);
}

// // int32_t usr_syscall_open(void)
// {

// }

// // int32_t usr_syscall_close(void)
// {

// }

int32_t usr_syscall_getpid(void)
{
    return syscall(SYSCALL__GETPID, 0, 0, 0);
}
