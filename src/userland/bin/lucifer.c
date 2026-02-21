#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usr_syscalls.h"

#define LINE_SIZE      128u
#define MAX_ARG_COUNT  8u

static int _fork_exec(const char *exec, const char *const argv[])
{
    int32_t status = usr_syscall_fork();
    if (status < 0) {
        printf("fork failed with status %d\n", status);
        return status;
    }

    if (status == 0) {
        status = usr_syscall_exec(exec, argv);
        printf("exec failed with status %d\n", status);  // exec never return on success
        return status;
    }
    else {
        const int32_t pid = status;
        int32_t wstatus = 0;
        status = usr_syscall_waitpid(pid, &wstatus);
        if (status < 0)
        {
            printf("waitpid failed with status %d\n", status);
        }
        return wstatus;
    }
}
int main(void)
{
    char line[LINE_SIZE] = "";  // raw input
    const char *tokens[1 + MAX_ARG_COUNT];

    const int32_t pid = usr_syscall_getpid();
    printf("[%u] welcome in lucifer shell\n", pid);

    for (;;)
    {
        printf("[%u] lucifer ~ ", pid);
        gets_s(line, LINE_SIZE);
        
        const size_t len = strlen(line);
        if (len == 0)
        {
            continue;
        }
        else if (0 == strcmp("exit", line))
        {
            printf("[%u] exiting lucifer shell !\n", pid);
            break;
        }

        unsigned int token_count = 0;
        const char *token = line;

        for (
            unsigned int i = 0u;
            i < LINE_SIZE && line[i] != '\0' && token_count < MAX_ARG_COUNT;
            i++
        ) {
            if (line[i] == ' ')
            {
                line[i] = '\0';
                if (strlen(token) > 0)
                {
                    tokens[token_count++] = token;
                }
                token = line + i + 1;
            }

        }
        if (strlen(token) > 0)
        {
            tokens[token_count++] = token;
        }
        tokens[token_count++] = NULL;

        //
        char exec_path[256];
        strcpy(exec_path, "/bin/");
        strcat(exec_path, tokens[0]);
        const char *const *argv = tokens + 1;

        const int status = _fork_exec(exec_path, argv);
        if (status != 0)
        {
            printf("[%u] child failed with status %d\n", pid, status);
        }
    }
    return 0;
}
