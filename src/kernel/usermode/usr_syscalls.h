#ifndef SATAN_USER_MODE_SYSCALLS_H_
#define SATAN_USER_MODE_SYSCALLS_H_

// perform syscalls rfom usermode
#include <stddef.h>
#include <stdint.h>

int32_t usr_syscall_yield(void);
int32_t usr_syscall_reboot(void);
int32_t usr_syscall_exit(int32_t status);
size_t usr_syscall_read(int32_t fd, void *data, size_t size);
size_t usr_syscall_write(int32_t fd, const void *data, size_t size);

int32_t usr_syscall_fork(void);

// int32_t usr_syscall_open(void);
// int32_t usr_syscall_close(void);
int32_t usr_syscall_getpid(void);

#endif
