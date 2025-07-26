
#include <stddef.h>
#include <stdint.h>

#include "lib.h"

// Serial Load Protocol
#define SERIAL_LOADER_INIT     0xFAu
#define SERIAL_LOADER_INIT_ACK 0xFBu
#define SERIAL_LOADER_END      0xFCu
#define SERIAL_LOADER_END_ACK  0xFDu

// Memory layout
#define KERNEL_START_ADRESS    0x8000u
#define KERNEL_MAX_SIZE        0x18000u // 96.0KiB

#define LOADER_START_ADDRESS   0x7000u
#define LOADER_MAX_SIZE        0x1000u

static uint32_t serial_load(uint8_t *buffer, uint32_t max_size)
{
    // wait for the transmision start signal
    while (loader_mini_uart_getc() != SERIAL_LOADER_INIT);
    loader_mini_uart_putc(SERIAL_LOADER_INIT_ACK);

    // receive size
    uint32_t datasize;
    loader_mini_uart_recv((uint8_t*)&datasize, sizeof(uint32_t));
    if (datasize > max_size) {
        return 0;
    }

    // receive buffer and send byte acks
    for (uint32_t i = 0u; i < datasize; i++) {
        const uint8_t byte = loader_mini_uart_getc();
        loader_mini_uart_putc(byte);
        buffer[i] = byte;
    }

    // send end signal
    loader_mini_uart_putc(SERIAL_LOADER_END);
    if (loader_mini_uart_getc() == SERIAL_LOADER_END_ACK)
    {
        return datasize;
    }
    else {
        // something bad happened
        return 0;
    }
}


// arguments for AArch32
void loader_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0,
    (void)r1,
    (void)atags;

    // Get the current program counter
    uint32_t pc;
    asm volatile ("adr %0, ." : "=r" (pc));

    if (pc > KERNEL_START_ADRESS)
    {
        // first run: copy the loader
        loader_memcpy(
            (void*)LOADER_START_ADDRESS,
            (void*)KERNEL_START_ADRESS,
            LOADER_MAX_SIZE);

        // execute the copied loader
        asm volatile("BX %0" : : "r"(LOADER_START_ADDRESS));
    }
    else
    {
        // we are in the loader copy.
        // load the kernel from mini UART
        loader_mini_uart_init();

        // try forever to reveive a kernel on serial
        while (serial_load(
            (uint8_t*)KERNEL_START_ADRESS,
            KERNEL_MAX_SIZE) == 0)
        {
            loader_cpu_delay(250);
        }

        // execute the kernel
        asm volatile("BX %0" : : "r"(KERNEL_START_ADRESS));
    }
}
