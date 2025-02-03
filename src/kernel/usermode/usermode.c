#include <stdint.h>

#include "usermode/usermode.h"
#include "usermode/usr_syscalls.h"
#include "usermode/libc/stdio.h"

#define CONTINUE car='\r';continue


void user_function(void)
{
    int32_t pid = usr_syscall_getpid();

    // starting user mode
    printf("[%u] welcome in user mode\n", pid);


    // const char msg[] = "Hello from stdout\r\n";
    // const int32_t status = usr_syscall_write(1, msg, 19);
    // printf("[%u] write status = %x\r\n", pid, status);

    // char buf[16];
    // const int32_t stat = usr_syscall_read(0, buf, 16);
    // buf[stat] = '\0';
    // printf("[%u] read status = %x, data = '%s'\r\n", pid, stat, buf);


    //
    char car = '\n';
    do {

        switch (car) {

            case '\n':
                printf("\n[%u] satan ~ ", pid);
                break;

            case 'x':
                printf("\n[%u] quit task now !!\n", pid);
                usr_syscall_exit(0);
                while (1); // hang.

            case 's':
            {
                printf("\n[%u] syscall YIELD\n", pid);
                const int32_t status = usr_syscall_yield();
                printf("[%u] syscall status: %x\n", pid, status);
                CONTINUE;
            }

            case 'q':
                printf("\n[%u] reboot now !!\n", pid);
                usr_syscall_reboot();
                while (1); // hang.

            case 'z':
            {
                printf("\n[%u] spawn new task\n", pid);
                const int32_t new_pid = usr_syscall_spawn((void*)user_function, 0);
                if (new_pid < 0)
                    printf("\n[%u] failed to spawn a task\n", pid);
                else
                    printf("\n[%u] spawned task: pid=%u\n", pid, new_pid);
            }
                break;

            default:
                putchar(car);
                break;

        }

        car = getchar();
    } while (1);
}
