#include <stddef.h>
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"

#include "usermode/user_init.h"

static const char *welcome_message =
"\x1b[31;1m\r\n"
"  █████████             █████                               ███████     █████████  \r\n"
" ███░░░░░███           ░░███                              ███░░░░░███  ███░░░░░███ \r\n"
"░███    ░░░   ██████   ███████    ██████   ████████      ███     ░░███░███    ░░░  \r\n"
"░░█████████  ░░░░░███ ░░░███░    ░░░░░███ ░░███░░███    ░███      ░███░░█████████  \r\n"
" ░░░░░░░░███  ███████   ░███      ███████  ░███ ░███    ░███      ░███ ░░░░░░░░███ \r\n"
" ███    ░███ ███░░███   ░███ ███ ███░░███  ░███ ░███    ░░███     ███  ███    ░███ \r\n"
"░░█████████ ░░████████  ░░█████ ░░████████ ████ █████    ░░░███████░  ░░█████████  \r\n"
" ░░░░░░░░░   ░░░░░░░░    ░░░░░   ░░░░░░░░ ░░░░ ░░░░░       ░░░░░░░     ░░░░░░░░░   \r\n"
"\x1b[0m";



#define USER_STACK 0x00800000u // 0x00800000 -> 0x00700000: 1 MB

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    const uint16_t cpu_mode = cpu_get_execution_mode();

    // initialize the mini UART
    mini_uart_init();

    // wait a first input
    mini_uart_puts(
        "[kernel] starting satan OS...\r\n"
            "[kernel] press a key...\r\n"
    );
    mini_uart_getc();

    // print a welcome message ;)
    mini_uart_puts(welcome_message);

    mini_uart_puts("\r\n[kernel] System informations:\r\n");
    mini_uart_puts("[kernel] OS   : satan\r\n");
    mini_uart_puts("[kernel] CPU  : arm1176jzf-s\r\n");
    mini_uart_puts("[kernel] GPU  : RTX 6090 Satanic Edition\r\n");
    mini_uart_puts("[kernel] RAM  : 42 Go\r\n");
    mini_uart_puts("[kernel] Temp : 666°C\r\n");
    mini_uart_puts("[kernel] Mode : 0x");
    mini_uart_put_hex(cpu_mode);
    mini_uart_puts("\r\n");

    // enable irq globaly
    cpu_irq_enable();

    // disable aux (mini uart) interuptions (using polling for now)
    mmio_write(REG__IRQ_DISABLE_1, IRQ1_AUX_INT);

    // start user mode
    mini_uart_puts("[kernel] call user mode !\r\n");
    start_usermode(USER_STACK);
}
