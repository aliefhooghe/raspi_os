#include <stdio.h>

#include "usr_syscalls.h"

int main(void)
{
    printf("reboot system...\n");
    usr_syscall_reboot();
    return 0;
}
