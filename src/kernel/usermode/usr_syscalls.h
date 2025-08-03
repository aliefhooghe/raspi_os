#ifndef SATAN_USER_MODE_SYSCALLS_H_
#define SATAN_USER_MODE_SYSCALLS_H_

#include <stddef.h>
#include <stdint.h>

#include "kernel_types.h"

//// User Mode syscall wrappers 

//
//  System
// 
int32_t usr_syscall_yield(void);
int32_t usr_syscall_reboot(void);
int32_t usr_syscall_exit(int32_t status);

//
//  Processes
// 
int32_t usr_syscall_fork(void);
int32_t usr_syscall_getpid(void);
int32_t usr_syscall_getppid(void);
int32_t usr_syscall_waitpid(int32_t pid, int32_t *wstatus);

//
//  Files
// 
int32_t usr_syscall_open(const char *path, int32_t flags, int32_t mode);
int32_t usr_syscall_close(int32_t fd);
size_t usr_syscall_read(int32_t fd, void *data, size_t size);
size_t usr_syscall_readdir(int32_t fd, dirent *entries, size_t count);
size_t usr_syscall_write(int32_t fd, const void *data, size_t size);
off_t usr_syscall_lseek(int fd, off_t offset, int whence);

int32_t usr_syscall_mount(const char *dev, const char *target, const char *fstype);


// return type is 0 or -1, _not_ a fd
int32_t usr_syscall_mknod(const char *path, mode_t mode, dev_t dev);

#endif
