#include <stdint.h>

#include "usermode/usermode.h"
#include "usermode/usr_syscalls.h"

#include "hardware/cpu.h"
#include "hardware/mini_uart.h"


static void print_cpu_mode(uint32_t id)
{
    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_printf("[%u] cpu mode: 0x%x", id, cpu_mode);

}

#define CONTINUE car='\r';continue


void user_function(void)
{
    int32_t pid = usr_syscall_getpid();

    // starting user mode
    mini_uart_printf("[%u] welcome in user mode\r\n", pid);
    print_cpu_mode(pid);

    const char msg[] = "Hello from stdout\r\n";
    const int32_t status = usr_syscall_write(1, msg, 19);
    mini_uart_printf("[%u] status = %x\r\n", pid, status);


    //
    char car = '\r';
    do {

        switch (car) {

            case '\r':
                mini_uart_printf("\r\n[%u] satan ~ ", pid);
                break;

            case 'x':
                mini_uart_printf("\r\n[%u] quit task now !!\r\n", pid);
                usr_syscall_exit(0);
                while (1); // hang.

            case 's':
            {
                mini_uart_printf("\r\n[%u] syscall YIELD\r\n", pid);
                const int32_t status = usr_syscall_yield();
                mini_uart_printf("[%u] syscall status: %x", pid, status);
                CONTINUE;
            }

            case 'p':
                print_cpu_mode(pid);
                CONTINUE;

            case 'q':
                mini_uart_printf("\r\n[%u] reboot now !!\r\n", pid);
                usr_syscall_reboot();
                while (1); // hang.

            case 'z':
            {
                mini_uart_printf("\r\n[%u] spawn new task\r\n", pid);
                const int32_t new_pid = usr_syscall_spawn((void*)user_function, 0);
                if (new_pid < 0)
                    mini_uart_printf("\r\n[%u] failed to spawn a task\r\n", pid);
                else
                    mini_uart_printf("\r\n[%u] spawned task: pid=%u\r\n", pid, new_pid);
            }
                break;

            default:
                mini_uart_putc(car);
                break;

        }

        car = mini_uart_getc();
    } while (1);
}
