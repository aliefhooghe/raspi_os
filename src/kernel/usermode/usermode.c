#include <stddef.h>
#include <stdint.h>

#include "usermode/usermode.h"
#include "usermode/usr_syscalls.h"
#include "usermode/libc/stdio.h"
#include "usermode/libc/string.h"
#include "hardware/mini_uart.h"


#define LINE_SIZE 128

void user_function(void)
{
    char line[LINE_SIZE] = "";

    FILE *stdout = get_stdout();
    const int32_t pid = usr_syscall_getpid();
    for (;;)
    {
        fprintf(stdout, "[%u] lucifer ~ ", pid);
        gets_s(line, LINE_SIZE);

        const size_t len = strlen(line);
        if (len == 0)
            continue;

        if (strcmp(line, "reboot") == 0)
        {
            fprintf(stdout, "[%u] reboot now !\n", pid);
            usr_syscall_reboot();
        }
        else if (strcmp(line, "switch") == 0)
        {
            fprintf(stdout, "[%u] switch to next process\n", pid);
            usr_syscall_yield();
        }
        else if (strcmp(line, "exit") == 0)
        {
            fprintf(stdout, "[%u] exit current process\n", pid);
            usr_syscall_exit(0u);
        }
        else
        {
            fprintf(stdout, "[%u] lucifer: %s: command not found\n", pid, line);
        }
    }
}
