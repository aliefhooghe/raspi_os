
// #include "stdio.h"

// #include "usr_syscalls.h"

#include "syscalls.h"

extern int32_t __syscall(uint32_t syscall_num, ...);

static inline int32_t syscall(uint32_t syscall_num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    const int32_t status = __syscall(syscall_num, arg0, arg1, arg2);
    asm volatile ("" ::: "memory"); // memory barrier
    return status;
}

int main(void)
{
    // (void)(argc);
    // (void)(argv);
    // FILE *stdout = fdopen(1, "w");

    // for (unsigned int i = 0; i < 4; i++)
    // fprintf(stdout, "Hello, World!\n");

    // for (int i = 0; i < 40; i++)

    for (int i = 0; i < 10; i++)  
        syscall(SYSCALL__YIELD, 0, 0, 0);
    return 41;
}
