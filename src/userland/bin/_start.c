//
// Userland Programs EntryPoint:
//
#include <stddef.h>
#include <stdint.h>

#include "usr_syscalls.h"
#include "stdio.h"

extern int main(int argc, const char *argv[]);

static int _argv_len(const char *argv[])
{
    int len = 0u;
    while (*argv++) len++;
    return len;
}

void _start(const char *argv[])
{
    // setup stdio
    stdout = fdopen(1, "w");

    // prepare standard C main arguments
    const int argc = _argv_len(argv);
    const int status = main(argc, argv);
    usr_syscall_exit(status);

    // unreachable
}
