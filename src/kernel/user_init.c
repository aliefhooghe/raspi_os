
#include "user_init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include <stdint.h>


void user_init(void)
{
    // Executed in user mode
    uint16_t cpu_mode = cpu_get_execution_mode();


    mini_uart_puts("[user] hello user mode world !!!\r\n");
    mini_uart_puts("[user] cpu mode: 0x");
    mini_uart_put_hex(cpu_mode);
    mini_uart_puts("\r\n");

    mini_uart_puts("[user] perform a syscall\r\n");
    const uint32_t status = syscall(3, 4);
    mini_uart_puts("[user] syscall status: ");
    mini_uart_put_int(status);
    mini_uart_puts("\r\n");

    cpu_mode = cpu_get_execution_mode();
    mini_uart_puts("[user] cpu mode: 0x");
    mini_uart_put_hex(cpu_mode);
    mini_uart_puts("\r\n");

    mini_uart_puts("[user] loop forever...\r\n");

    while (1);
}
