
#include "stdio.h"
#include "usr_syscalls.h"

int main(int argc, const char *argv[])
{
    usr_syscall_yield(0);
    fprintf(stdout, "Hello World !\nargc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        fprintf(stdout, " argv[%d] = '%s'\n", i,  argv[i]);
    }
    return 42;
}
