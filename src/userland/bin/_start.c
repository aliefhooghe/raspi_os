
// #include <stddef.h>
// #include <stdnoreturn.h>
#include "syscalls.h"

extern int main(void);


extern int32_t __syscall(uint32_t syscall_num, ...);

static inline int32_t syscall(uint32_t syscall_num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t status = __syscall(syscall_num, arg0, arg1, arg2);
    asm volatile ("" ::: "memory"); // memory barrier
    return status;
}

void _start(void)
{
    const int status = main();
    syscall(SYSCALL__EXIT, status, 0, 0);
    for (;;);
}
