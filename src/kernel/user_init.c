
#include "user_init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include "syscalls.h"
#include <stdint.h>

static void print_cpu_mode(void)
{
    const uint16_t cpu_mode = cpu_get_execution_mode();
    mini_uart_puts("[user] cpu mode: 0x");
    mini_uart_put_hex(cpu_mode);
}

#define CONTINUE car='\r';continue

void user_init(void)
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
                mini_uart_puts("\r\n[user] syscall DUMMY\r\n");
                const int32_t status = syscall(SYSCALL_DUMMY, 7, 5);
                mini_uart_puts("[user] syscall status: ");
                mini_uart_put_int(status);
                CONTINUE;
            }

            case 'p':
                mini_uart_puts("\r\n");
                print_cpu_mode();
                CONTINUE;

            case 'q':
                mini_uart_puts("\r\n[user] reboot now !!\r\n");
                syscall(SYSCALL_REBOOT, 0, 0);
                CONTINUE;

            default:
                mini_uart_putc(car);
                break;

        }

        car = mini_uart_getc();
    } while (1);

    //     if (car == '\r')
    //     {
    //         mini_uart_puts("\r\nsatan ~ ");
    //     }
    //     else if (car == 'o')
    //     {
    //         // const uint32_t code = syscall(3, 2);
    //         mini_uart_puts("\r\n enable aux (mini uart) interuptions ");
    //         // enable aux (mini uart) interuptions
    //         mmio_write(REG__IRQ_ENABLE_1, IRQ1_AUX_INT);
    //         while (1) {

    //         }
    //     }
    //     else if (car == 'q')
    //     {
    //         mini_uart_puts("\r\nreboot now !!\r\n");
    //         watchdog_init(0x100);
    //     }
    //     else {
    //         mini_uart_putc(car);
    //     }

    //     car = mini_uart_getc();
    // } while (1);


    // mini_uart_puts("[user] perform a syscall\r\n");
    // const int32_t status = syscall(SYSCALL_DUMMY, 7, 5);
    // mini_uart_puts("[user] syscall status: ");
    // mini_uart_put_int(status);
    // mini_uart_puts("\r\n");

    // cpu_mode = cpu_get_execution_mode();
    // mini_uart_puts("[user] cpu mode: 0x");
    // mini_uart_put_hex(cpu_mode);
    // mini_uart_puts("\r\n");

    // mini_uart_puts("[user] wait...\r\n");
    // cpu_delay(500000000);
    // cpu_delay(500000000);
    // cpu_delay(500000000);
    // cpu_delay(500000000);
    // cpu_delay(500000000);
    // cpu_delay(500000000);
    // cpu_delay(500000000);
    // mini_uart_puts("[user] reboot...\r\n");
    // syscall(SYSCALL_REBOOT, 7, 5);

    // while (1);
}


    // char car = '\r';
    // do {
    //     if (car == '\r')
    //     {
    //         mini_uart_puts("\r\nsatan ~ ");
    //     }
    //     else if (car == 'o')
    //     {
    //         // const uint32_t code = syscall(3, 2);
    //         mini_uart_puts("\r\n enable aux (mini uart) interuptions ");
    //         // enable aux (mini uart) interuptions
    //         mmio_write(REG__IRQ_ENABLE_1, IRQ1_AUX_INT);
    //         while (1) {

    //         }
    //     }
    //     else if (car == 'q')
    //     {
    //         mini_uart_puts("\r\nreboot now !!\r\n");
    //         watchdog_init(0x100);
    //     }
    //     else {
    //         mini_uart_putc(car);
    //     }

    //     car = mini_uart_getc();
    // } while (1);
