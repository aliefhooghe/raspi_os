#ifndef SATAN_USER_MODE_SYSCALLS_H_
#define SATAN_USER_MODE_SYSCALLS_H_

// perform syscalls rfom usermode
#include <stddef.h>
#include <stdint.h>


int32_t usr_syscall_yield(void);
int32_t usr_syscall_reboot(void);
int32_t usr_syscall_exit(int32_t status);

int32_t usr_syscall_fork(void);
int32_t usr_syscall_getpid(void);
int32_t usr_syscall_getppid(void);
int32_t usr_syscall_waitpid(int32_t pid, int32_t *wstatus);

int32_t usr_syscall_open(const char *path, int32_t flags, int32_t mode);
// int32_t usr_syscall_close(void);
size_t usr_syscall_read(int32_t fd, void *data, size_t size);
size_t usr_syscall_write(int32_t fd, const void *data, size_t size);

#endif
