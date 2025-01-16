#include <stddef.h>
#include <stdint.h>

#include "hardware/mini_uart.h"


#define UART_LOADER_MAGIC 0x42u
#define RX_BUFFER_SIZE    16384

static uint8_t rx_buffer[RX_BUFFER_SIZE] = {0};
static uint32_t size = 0u;

static uint32_t serial_load(uint8_t *buffer, uint32_t max_size)
{
    // receive and check magic
    const uint8_t magic = mini_uart_getc();
    if (magic != UART_LOADER_MAGIC)
    {
        return 0;
    }

    // receive size
    uint32_t datasize;
    mini_uart_recv((uint8_t*)&datasize, sizeof(uint32_t));
    if (datasize > max_size) {
        return 0;
    }

    // receive buffer and send byte acks
    for (uint32_t i = 0u; i < datasize; i++) {
        const uint8_t byte = mini_uart_getc();
        mini_uart_putc(byte);
        buffer[i] = byte;
    }

    // send final ack
    mini_uart_putc(UART_LOADER_MAGIC);

    return datasize;
}

// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    // initialize mini UART
    mini_uart_init();

    // load data from serial
    size = serial_load(rx_buffer, RX_BUFFER_SIZE);

    do {
        // print received data
        mini_uart_puts("loaded data from serial !!!!\n\r");
        mini_uart_puts((char*)rx_buffer);
    } while (1);
}
