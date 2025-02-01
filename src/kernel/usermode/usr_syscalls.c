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
    return syscall(SYSCALL_YIELD, 0, 0, 0);
}

int32_t usr_syscall_reboot(void)
{
    return syscall(SYSCALL_REBOOT, 0, 0, 0);
}

int32_t usr_syscall_spawn(void* proc_address, void* stack_address, uint32_t param)
{
    return syscall(SYSCALL_SPAWN, (uint32_t)proc_address, (uint32_t)stack_address, param);
}

int32_t usr_syscall_exit(int32_t status)
{
    return syscall(SYSCALL_EXIT, status, 0, 0);
}

// // int32_t usr_syscall_read(void)
// {

// }

// // int32_t usr_syscall_write(void)
// {

// }

// // int32_t usr_syscall_open(void)
// {

// }

// // int32_t usr_syscall_close(void)
// {

// }

int32_t usr_syscall_getpid(void)
{
    return syscall(SYSCALL_GETPID, 0, 0, 0);
}
