
#include <stddef.h>

#include "usr_syscalls.h"
#include "stdio.h"

extern int main(int argc, char **argv);

void _start(int argc, char **argv)
{
    // setup stdio
    stdout = fdopen(1, "w");

    // 
    const int status = main(argc, argv);
    usr_syscall_exit(status);

    // unreachable
}
