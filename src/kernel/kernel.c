#include <stddef.h>
#include <stdint.h>

#include "hardware/mini_uart.h"

static const char *welcome_message =
"  ________        __       ___________        __       _____  ___           ______      ________\r\n"
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

    // initialize the mini UART
    mini_uart_init();

    // print a welcome message ;)
    mini_uart_puts(welcome_message);

    char car = '\r';
    do {
        if (car == '\r')
        {
            mini_uart_puts("\r\nsatan ~ ");
        }
        else {
            mini_uart_putc(car);
        }

        car = mini_uart_getc();
    } while (1);
}
