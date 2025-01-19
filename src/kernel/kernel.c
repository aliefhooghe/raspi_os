#include <stddef.h>
#include <stdint.h>

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"
#include "hardware/watchdog.h"


static const char *welcome_message =
"\r\n  ________        __       ___________        __       _____  ___           ______      ________\r\n"
" /\"       )      /\"\"\\     (\"     _   \")      /\"\"\\     (\\\"   \\|\"  \\         /    \" \\    /\"\r\n"
"(:   \\___/      /    \\     )__/  \\\\__/      /    \\    |.\\\\   \\    |       // ____  \\  (:   \\___/\r\n"
" \\___  \\       /' /\\  \\       \\\\_ /        /' /\\  \\   |: \\.   \\\\  |      /  /    ) :)  \\___  \\\r\n"
"  __/  \\\\     //  __'  \\      |.  |       //  __'  \\  |.  \\    \\. |     (: (____/ //    __/  \\\\\r\n"
" /\" \\   :)   /   /  \\\\  \\     \\:  |      /   /  \\\\  \\ |    \\    \\ |      \\        /    /\" \\   :)\r\n"
"(_______/   (___/    \\___)     \\__|     (___/    \\___) \\___|\\____\\)       \\\"_____/    (_______/\r\n"
;

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    const uint16_t cpu_mode = cpu_get_execution_mode();

    // disable aux (mini uart) interuptions
    mmio_write(REG__IRQ_DISABLE_1, IRQ1_AUX_INT);

    // initialize the mini UART
    mini_uart_init();

    // wait a first input
    mini_uart_puts("press a key...");
    mini_uart_getc();

    // print a welcome message ;)
    mini_uart_puts(welcome_message);

    mini_uart_puts("\r\nSystem informations:\r\n");
    mini_uart_puts("OS   : satan\r\n");
    mini_uart_puts("CPU  : arm1176jzf-s\r\n");
    mini_uart_puts("GPU  : RTX 4090\r\n");
    mini_uart_puts("RAM  : 42 To\r\n");
    mini_uart_puts("Temp : 666°C\r\n");
    mini_uart_puts("Mode : 0x");
    mini_uart_put_hex(cpu_mode);
    mini_uart_puts("\r\n\r\n");

    // enable irq globaly
    cpu_irq_enable();


    char car = '\r';
    do {
        if (car == '\r')
        {
            mini_uart_puts("\r\nsatan ~ ");
        }
        else if (car == 'o')
        {
            // const uint32_t code = syscall(3, 2);
            mini_uart_puts("\r\n enable aux (mini uart) interuptions ");
            // enable aux (mini uart) interuptions
            mmio_write(REG__IRQ_ENABLE_1, IRQ1_AUX_INT);
            while (1) {

            }
        }
        else if (car == 'q')
        {
            mini_uart_puts("\r\nreboot now !!\r\n");
            watchdog_init(0x100);
        }
        else {
            mini_uart_putc(car);
        }

        car = mini_uart_getc();
    } while (1);
}
