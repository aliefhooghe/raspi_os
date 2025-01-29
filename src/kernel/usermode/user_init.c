
#include "user_init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include "syscalls.h"
#include <stdint.h>

static void print_cpu_mode(void)
{
    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_printf("[user] cpu mode: 0x%x", cpu_mode);

}

#define CONTINUE car='\r';continue

void user_function(void)
{
    // starting user mode
    mini_uart_puts("[user] welcome in user mode\r\n");
    print_cpu_mode();

    //

    char car = '\r';
    do {

        switch (car) {

            case '\r':
                mini_uart_puts("\r\n[user] satan ~ ");
                break;

            case 's':
            {
                mini_uart_puts("\r\n[user] syscall YIELD\r\n");
                const int32_t status = syscall(SYSCALL_YIELD, 0, 0);
                mini_uart_puts("[user] syscall status: ");
                mini_uart_put_uint(status);
                CONTINUE;
            }

            case 'p':
                mini_uart_puts("\r\n");
                print_cpu_mode();
                CONTINUE;

            case 'q':
                mini_uart_puts("\r\n[user] reboot now !!\r\n");
                syscall(SYSCALL_REBOOT, 0, 0);
                while (1); // hang.

            default:
                mini_uart_putc(car);
                break;

        }

        car = mini_uart_getc();
    } while (1);
}
