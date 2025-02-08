#ifndef SATAN_SYSCALLS_H_
#define SATAN_SYSCALLS_H_

#include <stdint.h>

typedef enum {
    SYSCALL_YIELD = 0,
    SYSCALL_REBOOT,
    SYSCALL_SPAWN,
    SYSCALL_EXIT,

    SYSCALL_READ,
    SYSCALL_WRITE,
    // SYSCALL_OPEN,
    // SYSCALL_CLOSE,

    // pour un shell dup, dup2, exec, fork, getppid (parent pid), wait

    // SYSCALL_EXEC,
    // SYSCALL_FORK,
    // SYSCALL_WAIT
    // SYSCALL_PIPE


    SYSCALL_GETPID,
    // SYSCALL_GETPPID,

    SYSCALL_COUNT
} syscall_num_t;

#define SYSCALL_STATUS_OK   0
#define SYSCALL_STATUS_ERR  (-1)

/**
exit: int32_t
read:  uint32_t fd  , const void *data  , uint32_t size
write: uint32_t fd  , void *data        , uint32_t size
open: const char*   , uint32_t flags    , uint32_t mode
close: uint32_t fd

poll ?
seek ?

 */

#endif
