#include <stddef.h>
#include <stdint.h>

#include "usermode/usermode.h"
#include "usermode/usr_syscalls.h"
#include "usermode/libc/stdio.h"
#include "usermode/libc/string.h"


#define LINE_SIZE 128


static void test_fork(int32_t pid, FILE *stdout)
{
    fprintf(stdout, "[%u] process is about to fork !\n", pid);
    const uint32_t status = usr_syscall_fork();

    if (status == 0u)
    {
        const uint32_t cpid = usr_syscall_getpid();
        fprintf(stdout, "we are in the child pid=%u, ppid=%u\n", cpid, pid);

        for (uint32_t i = 0u; i < 4u; i++)
        {
            fprintf(stdout, "[%u] child process is working iteration=%u\n", cpid, i);
        }

        fprintf(stdout, "[%u] exit child process.\n", cpid);
        usr_syscall_exit(0u);
    }
    else
    {
        const uint32_t ppid = usr_syscall_getpid();
        fprintf(stdout, "we are in the parent ppid=%u, pid_arg=%u, child pid=%u\n", ppid, pid, status);
    }
}

void user_function(void)
{
    DECLARE_STDOUT;

    char line[LINE_SIZE] = "";
    const int32_t pid = usr_syscall_getpid();
    fprintf(stdout, "[%u] start usermode function", pid);

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
        else if (strcmp(line, "fork") == 0)
        {
            fprintf(stdout, "[%u] call fork test procedure:\n", pid);
            test_fork(pid, stdout);
        }
        else
        {
            fprintf(stdout, "[%u] lucifer: %s: command not found\n", pid, line);
        }
    }
}
