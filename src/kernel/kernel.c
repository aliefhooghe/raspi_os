#include <stddef.h>
#include <stdint.h>

#include "user_init.h"

#include "hardware/cpu.h"
#include "hardware/interupts.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"
#include "hardware/mmio.h"


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

    // initialize the mini UART
    mini_uart_init();

    // wait a first input
    mini_uart_puts("[kernel] starting satan OS...\r\n[kernel] press a key...");
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
    start_usermode();
}
