
#include <stddef.h>
#include <stdnoreturn.h>
#include "usr_syscalls.h"

extern int main(int argc, char **argv);

noreturn void _start(void)
{
    const int status = main(0, NULL);
    usr_syscall_exit(status);
}
