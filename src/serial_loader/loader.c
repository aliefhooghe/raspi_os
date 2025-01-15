#include <stddef.h>
#include <stdint.h>

#include "hardware/mini_uart.h"

// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    // initialize UART for Raspi0
    mini_uart_init();
    mini_uart_puts("serial loader ready.\r\n");

    do {
        (void)(volatile uint32_t)mini_uart_getc();
    } while (1);
}
