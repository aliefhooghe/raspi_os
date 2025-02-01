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

// uint32_t new_stack_address_by_pid(uint32_t id)
// {
//     return 0x00800000u - id * 0x00100000u;
// }

void user_function(void)
{
    int32_t pid = usr_syscall_getpid();
    int spawned = 0;

    // starting user mode
    mini_uart_printf("[%u] welcome in user mode\r\n", pid);
    print_cpu_mode(pid);

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
                mini_uart_printf(" \r\n(%u)\r\n", spawned);
                print_cpu_mode(pid);
                CONTINUE;

            case 'q':
                mini_uart_printf("\r\n[%u] reboot now !!\r\n", pid);
                usr_syscall_reboot();
                while (1); // hang.

            case 'z':
            {
                if (!spawned)
                {
                    spawned = 1;
                    mini_uart_printf("\r\n[%u] spawn new task\r\n", pid);
                    const int32_t status = usr_syscall_spawn((void*)user_function, 0);
                    mini_uart_printf("\r\n[%u] spawn syscall status: %x\r\n", pid, status);
                }
                else {
                    mini_uart_printf("\r\n[%u] no spawn !!\r\n", pid);
                }
            }
                break;

            default:
                mini_uart_putc(car);
                break;

        }

        car = mini_uart_getc();
    } while (1);
}
