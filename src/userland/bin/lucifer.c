
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/kernel/lib/str.h"
#include "usr_syscalls.h"

#define LINE_SIZE 128u

__attribute__((noinline)) void ls(FILE *stdout, const char *path)
{
    fprintf(stdout, "list file in path: %s\n", path);
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stdout, "directory %s does not exists\n", path);
        return;
    }

    struct dirent *entity = NULL;
    while ((entity = readdir(dir))) {
        fprintf(stdout,
            " %s (%s)\n",
            entity->d_name,
                entity->d_type == DT_DIR ? "dir" :
                entity->d_type == DT_REG ? "reg" :
                entity->d_type == DT_CHR ? "character device" :
                entity->d_type == DT_BLK ? "block device" :
                "unknown");
    }

    closedir(dir);
}

__attribute__((noinline)) void test_fork(int32_t pid, FILE *stdout)
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

__attribute__((noinline)) void test_w_file(FILE *stdout)
{
    const int s1 = usr_syscall_mkdir("/test", S_IFDIR);
    fprintf(stdout, "mkdir: status=%x\n", s1);

    int fd = usr_syscall_open("/test/toto.txt", O_CREAT, S_IFREG);
    if (fd < 0)
    {
        fprintf(stdout, "open failed.\n");
        return;
    }
    
    FILE *test = fdopen(fd, "w");
    if (test == NULL)
    {
        fprintf(stdout, "fdopen failed.\n");
        return;
    }
    
    fprintf(test, "hello THE world\n");

    const int s2 = fclose(test);
    fprintf(stdout, "fclose: status=%x\n", s2);
}

__attribute__((noinline)) void test_r_file(FILE *stdout, const char *path)
{
    int fd = usr_syscall_open(path, 0u, S_IFREG);
    if (fd < 0)
    {
        fprintf(stdout, "open failed.\n");
        return;
    }
    
    FILE *test = fdopen(fd, "r");
    if (test == NULL)
    {
        fprintf(stdout, "fdopen failed.\n");
        return;
    }

    char buffer[128];
    const ssize_t sz = fread(buffer, 1, 127, test);
    if (sz < 0)
    {
        fprintf(stdout, "fread failed\n");
        return; 
    }
    buffer[sz] = '\0';
    
    fprintf(stdout, "file content is:\n%s\n----\n");

    const int s2 = fclose(test);
    fprintf(stdout, "fclose: status=%x\n", s2);
}

__attribute__((noinline)) void fork_exec(FILE *stdout, const char *exec)
{
    fprintf(stdout, "fork !");
    const uint32_t status = usr_syscall_fork();

    if (status < 0) {
        fprintf(stdout, "fork failed\n");
        return;
    }

    if (status == 0) {
        fprintf(stdout, "in child: exec %s\n", exec);
        usr_syscall_exec(exec);
        usr_syscall_exit(0);
    }
    else {
        for (int i = 0; i < 2;i++) {
            fprintf(stdout, "in parent: yield %u\n", i);
            usr_syscall_yield();
        }
        fprintf(stdout, "in parent: wait child %u\n", status);
        int32_t wstatus = 0;
        usr_syscall_waitpid(status, &wstatus);
        fprintf(stdout, "from parent: child exited with status %u\n", wstatus);
    }
}

__attribute__((noinline)) void test_read_data(FILE *stdout)
{
    char buffer[256] = "";
    FILE *f = fopen("/data/text.txt", "r");

    if (f == NULL) {
        fprintf(stdout, "failed to open data file\n");
        return;
    }

    const int status = fread(buffer, 256, 1, f);
    if (status < 0) {
        fprintf(stdout, "error reading fiile\n");
    }
    else {
        buffer[status] = '\0';
        fprintf(stdout, "data: %s\n", buffer);
    }

    fclose(f);
}
__attribute__((noinline)) void test_read_dataa(FILE *stdout)
{
    char buffer[256] = "";
    int fd = usr_syscall_open("/data/text.txt", 0, 0);

    if (fd < 0 ) {
        fprintf(stdout, "failed to open data file\n");
        return;
    }

    const int status = usr_syscall_read(fd, buffer, 256);
    if (status < 0) {
        fprintf(stdout, "error reading fiile\n");
    }
    else {
        buffer[status] = '\0';
        fprintf(stdout, "data: %s\n", buffer);
    }

    usr_syscall_close(fd);
}

int main(void)
{
    char line[LINE_SIZE] = "";
    const int32_t pid = usr_syscall_getpid();
    fprintf(stdout, "[%u] welcome in lucifer shell\n", pid);

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
        else if (strcmp(line, "loop") == 0) {
            for (int i = 0; i < 256;i++) {
                fprintf(stdout, "yield %u\n", i);
                const int status = usr_syscall_yield(i);
                if (status != i) {
                    fprintf(stdout, "=> invalid return status %d instead of %d", status, i);
                }
            }
            fprintf(stdout, "done\n");
        }
        else if (strcmp(line, "switch") == 0)
        {
            fprintf(stdout, "[%u] switch to next process\n", pid);
            const int status = usr_syscall_yield(42);
            fprintf(stdout, "[%u] switch returned %d\n", pid, status);
            
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
        else if (strcmp(line, "write") == 0)
        {
            fprintf(stdout, "[%u] call write test proc:\n", pid);
            test_w_file(stdout);
        }
        else if (strcmp(line, "read") == 0)
        {
            fprintf(stdout, "[%u] call read test proc:\n", pid);
            test_r_file(stdout, "/test/toto.txt");
        }
        else if (strcmp(line, "elf") == 0)
        {
            fprintf(stdout, "[%u] test elf loader:\n", pid);
            fork_exec(stdout, "/bin/hello");
        }
        else if ('l' == line[0] && 's' == line[1])
        {
            const char *arg = line + 3;

            if (line[2] == '\0')
            {
                arg = "/";
            }
            else if (strlen(line) <= 3 || line[2] != ' ') {
                fprintf(stdout, "ls: bad syntax\n");
                continue;
            }

            ls(stdout, arg);
        }
        else if (strcmp(line, "mkdir") == 0)
        {
            fprintf(stdout, "[%u] test mkdir on /data\n", pid);
            const int status = usr_syscall_mkdir("/data", S_IFDIR);
            fprintf(stdout, "[%u] mkdir /data => return 0x%x\n", pid, status);
            ls(stdout, "/");
        }
        else if (strcmp(line, "zz") == 0) {
            test_read_data(stdout);
        }
        else if (strcmp(line, "ed") == 0) {
            test_read_dataa(stdout);
        }
        else
        {
            fprintf(stdout, "[%u] lucifer: %s: command not found\n", pid, line);
        }
    }
    return 0;
}
