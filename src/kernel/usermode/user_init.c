
#include "user_init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include "syscalls.h"
#include <stdint.h>

static void print_cpu_mode(const char *name)
{
    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_printf("[%s] cpu mode: 0x%x", name, cpu_mode);

}

#define CONTINUE car='\r';continue

static volatile int _spawned = 0;

static void user_function(const char *name)
{
    // starting user mode
    mini_uart_printf("[%s] welcome in user mode\r\n", name);
    print_cpu_mode(name);

    //
    char car = '\r';
    do {

        switch (car) {

            case '\r':
                mini_uart_printf("\r\n[%s] satan ~ ", name);
                break;

            case 's':
            {
                mini_uart_printf("\r\n[%s] syscall YIELD\r\n", name);
                const int32_t status = syscall(SYSCALL_YIELD, 0x42, 0xFA);
                mini_uart_printf("[%s] syscall status: %x", name, status);
                CONTINUE;
            }

            case 'p':
                mini_uart_puts("\r\n");
                print_cpu_mode(name);
                CONTINUE;

            case 'q':
                mini_uart_printf("\r\n[%s] reboot now !!\r\n", name);
                syscall(SYSCALL_REBOOT, 0, 0);
                while (1); // hang.

            case 'z':
                if (!_spawned) {
                    mini_uart_printf("\r\n[%s] spawn another task !\r\n", name);
                    const int32_t status = syscall(SYSCALL_SPAWN, (uint32_t)user_function2, 0x00700000u);
                    mini_uart_printf("\r\n[%s] spawn syscall status: %x\r\n", name, status);
                    _spawned = 1;
                }
                else {
                    mini_uart_printf("\r\n[%s] ignore spawn request", name);
                }
                break;

            default:
                mini_uart_putc(car);
                break;

        }

        car = mini_uart_getc();
    } while (1);
}

void user_function1(void)
{
    user_function("user app1");
}

void user_function2(void)
{
    user_function("user app2");
}
