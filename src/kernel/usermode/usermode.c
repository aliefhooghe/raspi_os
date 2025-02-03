#include <stddef.h>
#include <stdint.h>

#include "usermode/usermode.h"
#include "usermode/usr_syscalls.h"
#include "usermode/libc/stdio.h"
#include "usermode/libc/string.h"


#define LINE_SIZE 128

void user_function(void)
{
    char line[LINE_SIZE] = "";
    const int32_t pid = usr_syscall_getpid();

    printf("[%u] welcome in user mode process !!\n", pid);

    for (;;)
    {
        printf("satan ~ ");
        gets_s(line, LINE_SIZE);

        const size_t len = strlen(line);
        if (len > 0)
            printf("you wrote '%s'\n", line);
    }

    // do {

    //     switch (car) {

    //         case '\n':
    //             printf("\n[%u] satan ~ ", pid);
    //             break;

    //         case 'x':
    //             printf("\n[%u] quit task now !!\n", pid);
    //             usr_syscall_exit(0);
    //             while (1); // hang.

    //         case 's':
    //         {
    //             printf("\n[%u] syscall YIELD\n", pid);
    //             const int32_t status = usr_syscall_yield();
    //             printf("[%u] syscall status: %x\n", pid, status);
    //             CONTINUE;
    //         }

    //         case 'q':
    //             printf("\n[%u] reboot now !!\n", pid);
    //             usr_syscall_reboot();
    //             while (1); // hang.

    //         case 'z':
    //         {
    //             printf("\n[%u] spawn new task\n", pid);
    //             const int32_t new_pid = usr_syscall_spawn((void*)user_function, 0);
    //             if (new_pid < 0)
    //                 printf("\n[%u] failed to spawn a task\n", pid);
    //             else
    //                 printf("\n[%u] spawned task: pid=%u\n", pid, new_pid);
    //         }
    //             break;

    //         default:
    //             putchar(car);
    //             break;

    //     }

    //     car = getchar();
    // } while (1);
}
