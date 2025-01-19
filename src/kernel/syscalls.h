#ifndef SATAN_SYSCALLS_H_
#define SATAN_SYSCALLS_H_

#include <stdint.h>

typedef enum {
    SYSCALL_DUMMY = 0,
    SYSCALL_REBOOT = 1,
    SYSCALL_COUNT
} syscall_num_t;

#define SYSCALL_STATUS_OK   0
#define SYSCALL_STATUS_ERR  (-1)

int32_t kernel_syscall_handler(
    syscall_num_t syscall_num,
    uint32_t arg0, uint32_t arg1
);

#endif
