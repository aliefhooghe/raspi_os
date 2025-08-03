#include <stddef.h>
#include <stdint.h>

#include "usermode/usermode.h"
#include "kernel_types.h"
#include "usermode/usr_syscalls.h"
#include "usermode/libc/stdio.h"
#include "usermode/libc/string.h"
#include "usermode/libc/dirent.h"

#define LINE_SIZE 128


static void ls(FILE *stdout, const char *path)
{
    fprintf(stdout, "list file in path: %s\n", path);
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stdout, "directory %s does not exists\n", path);
        return;
    }
    else
    {
        fprintf(stdout, "directory %s was oppened", path);
    }

    struct dirent *entity = NULL;
    while ((entity = readdir(dir))) {
        fprintf(stdout,
            "- %s (%s)\n",
            entity->d_name,
                entity->d_type == DT_DIR ? "dir" :
                entity->d_type == DT_REG ? "reg" :
                entity->d_type == DT_CHR ? "character device" :
                "unknown");
    }

    fprintf(stdout, "list directory %s: done", path);
    closedir(dir);
}

static void test_fork(int32_t pid, FILE *stdout)
{
    fprintf(stdout, "[%u] process is about to fork !\n", pid);
    const uint32_t status = usr_syscall_fork();

    if (status == 0u)
    {
        const uint32_t cpid = usr_syscall_getpid();
        fprintf(stdout, "[%u] we are in the child pid=%u, ppid=%u\n", cpid, cpid, pid);

        for (uint32_t i = 0u; i < 16u; i++)
        {
            fprintf(stdout, "[%u] child process is working iteration=%u\n", cpid, i);
        }

        // VFS tests
        FILE *file = fopen("/dev/tty", "w");
        fprintf(file, "[%u] WRITE TO TTY\n", cpid);
        fflush(file);
        fclose(file);

        //
        int fd2 = usr_syscall_open("/toto/tata.txt", 0u, 0u);
        if (fd2 == -1)
        {
            fprintf(stdout, "[%u] could not open toto/tata.txt\n", cpid);
        }
        else
        {
            fprintf(stdout, "[%u] HOOORIBLE", cpid);
        }

        fprintf(stdout, "[%u] exit child process.\n", cpid);
        usr_syscall_exit(42u);
    }
    else
    {
        pid = usr_syscall_getpid();

        fprintf(stdout, "[%u] we are in the parent pid=%u, child pid=%u\n", pid, pid, status);
        fprintf(stdout, "[%u] wait child: pid=%u\n", pid, status);

        int32_t wstatus = 0u;
        const int32_t ret = usr_syscall_waitpid(status, &wstatus);

        fprintf(stdout, "[%u] waitpid returned %u. child exited with status=%u\n", pid, ret, wstatus);
    }

    fprintf(stdout, "[%u] exit fork test function\n", usr_syscall_getpid());
}

void user_function(void)
{
    FILE *stdout = fdopen(1, "w");
    
    char line[LINE_SIZE] = "";
    const int32_t pid = usr_syscall_getpid();
    fprintf(stdout, "[%u] start usermode function\n", pid);

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
        else if (strcmp(line, "ls") == 0)
        {
            fprintf(stdout, "[%u] call ls test procedure:\n", pid);
            ls(stdout, "/");
            ls(stdout, "/dev");
        }
        else
        {
            fprintf(stdout, "[%u] lucifer: %s: command not found\n", pid, line);
        }
    }
}
