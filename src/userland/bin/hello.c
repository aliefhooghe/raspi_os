
#include "stdio.h"
#include "usr_syscalls.h"

int main(void)
{
    usr_syscall_yield(0);
    fprintf(stdout, "Hello World !\n");
    return 42;
}
