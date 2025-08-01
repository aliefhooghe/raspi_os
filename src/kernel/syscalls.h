#ifndef SATAN_SYSCALLS_H_
#define SATAN_SYSCALLS_H_

#include <stdint.h>

//
// Syscall enumerations
// 
#define FOREACH_SYSCALL(SYSCALL) \
  SYSCALL(YIELD)                 \
  SYSCALL(REBOOT)                \
  SYSCALL(EXIT)                  \
  SYSCALL(OPEN)                  \
  SYSCALL(CLOSE)                 \
  SYSCALL(READ)                  \
  SYSCALL(WRITE)                 \
  SYSCALL(LSEEK)                 \
  SYSCALL(FORK)                  \
  SYSCALL(WAITPID)               \
  SYSCALL(GETPPID)               \
  SYSCALL(GETPID)

#define GENERATE_ENUM(ENUM) SYSCALL__##ENUM,
typedef enum {
  FOREACH_SYSCALL(GENERATE_ENUM)
  SYSCALL_COUNT
} syscall_num_t;

//
// Default Syscall Return statuses
// 
#define SYSCALL_STATUS_OK   0
#define SYSCALL_STATUS_ERR  (-1)

#endif
