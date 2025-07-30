#ifndef SATAN_SYSCALLS_H_
#define SATAN_SYSCALLS_H_

#include <stdint.h>

// Syscall names
#define FOREACH_SYSCALL(SYSCALL) \
  SYSCALL(YIELD)                 \
  SYSCALL(REBOOT)                \
  SYSCALL(EXIT)                  \
  SYSCALL(OPEN)                  \
  SYSCALL(CLOSE)                  \
  SYSCALL(READ)                  \
  SYSCALL(WRITE)                 \
  SYSCALL(FORK)                  \
  SYSCALL(WAITPID)               \
  SYSCALL(GETPPID)               \
  SYSCALL(GETPID)

// pour un shell dup, dup2, exec, wait


#define GENERATE_ENUM(ENUM) SYSCALL__##ENUM,
typedef enum {
  FOREACH_SYSCALL(GENERATE_ENUM)
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
