
#include "stdio.h"
#include "usr_syscalls.h"

int main(int argc, const char *argv[])
{
    usr_syscall_yield(0);
    printf("Hello World !\nargc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf(" argv[%d] = '%s'\n", i,  argv[i]);
    }
    return 42;
}
