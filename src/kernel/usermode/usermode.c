#include <stdint.h>

#include "usermode.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include "syscalls.h"

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
    // starting user mode
    mini_uart_printf("[%u] welcome in user mode\r\n", id);
    print_cpu_mode(id);

    //
    char car = '\r';
    do {

        switch (car) {

            case '\r':
                mini_uart_printf("\r\n[%u] satan ~ ", id);
                break;

            case 's':
            {
                mini_uart_printf("\r\n[%u] syscall YIELD\r\n", id);
                const int32_t status = syscall(SYSCALL_YIELD, 0x42, 0xFA, 0);
                mini_uart_printf("[%u] syscall status: %x", id, status);
                CONTINUE;
            }

            case 'p':
                mini_uart_puts("\r\n");
                print_cpu_mode(id);
                CONTINUE;

            case 'q':
                mini_uart_printf("\r\n[%u] reboot now !!\r\n", id);
                syscall(SYSCALL_REBOOT, 0, 0, 0);
                while (1); // hang.

            case 'z':
            {
                if (id == 0)
                {
                    const uint32_t new_task_id = id + 1u;
                    const uint32_t new_task_stack = stack_address_by_id(new_task_id);
                    mini_uart_printf("\r\n[%u] spawn task %u\r\n", id, new_task_id);
                    const int32_t status = syscall(
                        SYSCALL_SPAWN,
                        (uint32_t)user_function,
                        new_task_stack,
                        new_task_id);
                    mini_uart_printf("\r\n[%u] spawn syscall status: %x\r\n", id, status);
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
