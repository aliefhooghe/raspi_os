#include <stdint.h>

#include "usermode/usr_syscalls.h"
#include "syscalls.h"

// perform a syscall
extern int32_t syscall(uint32_t syscall_num, ...);


int32_t usr_syscall_yield(void)
{
    return syscall(SYSCALL_YIELD);
}

int32_t usr_syscall_reboot(void)
{
    return syscall(SYSCALL_REBOOT);
}

int32_t usr_syscall_spawn(void* proc_address, void* stack_address, uint32_t param)
{
    return syscall(SYSCALL_SPAWN, proc_address, stack_address, param);
}

int32_t usr_syscall_exit(int32_t status)
{
    return syscall(SYSCALL_EXIT, status);
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
    return syscall(SYSCALL_GETPID);
}
