#include <stdio.h>
#include "usr_syscalls.h"

static int32_t _fork_exec_wait(const char *exec)
{
    const int32_t status = usr_syscall_fork();
    if (status < 0) {
        fprintf(stdout, "init: fork failed\n");
        return status;
    }
    if (status == 0) {
        const int32_t exec_status = usr_syscall_exec(exec);
        if (exec_status < 0)
            fprintf(stdout, "init: exec failed\n");
        // we should only reach this code when exec failed
        usr_syscall_exit(exec_status);
    }
    else {
        int32_t wstatus = 0;
        usr_syscall_waitpid(status, &wstatus);
        fprintf(stdout, "init: child exited with status %u\n", wstatus);
        return wstatus;
    }

    // should be unreachable
    return -1;
}

int main(void)
{
    for (;;) {
        fprintf(stdout, "init: starting lucifer shell !!\n");
        _fork_exec_wait("/bin/lucifer");
    }   
    return 0;
}
