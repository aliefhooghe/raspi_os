
#include <stddef.h>

#include "usr_syscalls.h"
#include "stdio.h"

extern int main(void);

void _start(void)
{
    // setup stdio
    stdout = fdopen(1, "w");

    // 
    const int status = main();
    usr_syscall_exit(status);

    // unreachable
}
