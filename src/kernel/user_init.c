
#include "user_init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include "syscalls.h"
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
    const int32_t status = syscall(SYSCALL_DUMMY, 7, 5);
    mini_uart_puts("[user] syscall status: ");
    mini_uart_put_int(status);
    mini_uart_puts("\r\n");

    cpu_mode = cpu_get_execution_mode();
    mini_uart_puts("[user] cpu mode: 0x");
    mini_uart_put_hex(cpu_mode);
    mini_uart_puts("\r\n");

    mini_uart_puts("[user] wait...\r\n");
    cpu_delay(500000000);
    cpu_delay(500000000);
    cpu_delay(500000000);
    cpu_delay(500000000);
    cpu_delay(500000000);
    cpu_delay(500000000);
    cpu_delay(500000000);
    mini_uart_puts("[user] reboot...\r\n");
    syscall(SYSCALL_REBOOT, 7, 5);

    while (1);
}
