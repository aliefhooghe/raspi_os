#ifndef SATAN_USER_MODE_SYSCALLS_H_
#define SATAN_USER_MODE_SYSCALLS_H_

// perform syscalls rfom usermode
#include <stdint.h>

int32_t usr_syscall_yield(void);
int32_t usr_syscall_reboot(void);
int32_t usr_syscall_spawn(void* proc_address, void* stack_address, uint32_t param);
int32_t usr_syscall_exit(int32_t status);
// int32_t usr_syscall_read(void);
// int32_t usr_syscall_write(void);
// int32_t usr_syscall_open(void);
// int32_t usr_syscall_close(void);
int32_t usr_syscall_getpid(void);

#endif
