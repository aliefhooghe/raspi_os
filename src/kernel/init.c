
#include "init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/mini_uart.h"
#include <stdint.h>

extern unsigned int global_irq_counter;

void init(void)
{
    while (1)
    {

        // syscall(1, 2);

        // const uint8_t car = mini_uart_getc();

        // mini_uart_puts("from kernel: irq_counter = ");
        // // mini_uart_put_int(global_irq_counter);
        // mini_uart_puts("\r\n");
        // mini_uart_putc(car);
        // mini_uart_puts("\r\n");
    }
}
