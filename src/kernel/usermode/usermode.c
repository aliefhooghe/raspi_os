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

uint32_t stack_address_by_id(uint32_t id)
{
    return 0x00800000u - id * 0x00100000u;
}

void user_function(uint32_t id)
{
    mini_uart_printf("-- start user function ---\r\n");
    int32_t pid = id; //usr_syscall_getpid();

    // starting user mode
    mini_uart_printf("[%u] welcome in user mode\r\n", id);
    print_cpu_mode(id);

    //
    char car = '\r';
    do {

        switch (car) {

            case '\r':
                mini_uart_printf("\r\n[%u] satan ~ ", pid);
                break;

            case 's':
            {
                mini_uart_printf("\r\n[%u] syscall YIELD\r\n", pid);
                const int32_t status = usr_syscall_yield();
                mini_uart_printf("[%u] syscall status: %x", pid, status);
                CONTINUE;
            }

            case 'p':
                mini_uart_puts("\r\n");
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
                    const uint32_t new_task_stack = new_stack_address_by_id(pid);
                    mini_uart_printf("\r\n[%u] spawn new task with stack = %x\r\n", pid, new_task_stack);
                    const int32_t status = usr_syscall_spawn((void*)user_function, (void*)new_task_stack, 0);
                    mini_uart_printf("\r\n[%u] spawn syscall status: %x\r\n", pid, status);
                    spawned = 1;
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
