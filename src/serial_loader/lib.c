
#include "lib.h"
#include <stdint.h>

void *loader_memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size)
{
    uint8_t *dst = (uint8_t*)destination;
    const uint8_t *src = (const uint8_t*)source;

    for (uint32_t i = 0; i < size; ++i) {
        dst[i] = src[i];
    }

    return destination;
}

// defined in lib.S
extern void loader_mmio_write(uintptr_t addr, uint32_t value);
extern uint32_t loader_mmio_read(uintptr_t addr);

// Registers for the GPIOs
#define GPFSEL1     0x20200004  // GPIO Function Select Register (for GPIO14 and GPIO15)
#define GPSET0      0x2020001C  // GPIO Pin Output Set Register
#define GPCLR0      0x20200028  // GPIO Pin Output Clear Register
#define GPPUD       0x20200094  // GPIO Pull-up/down Register
#define GPPUDCLK0   0x20200098  // GPIO Pull-up/down Clock Register

// Registers for the mini UART (AUX_MU)
#define AUX_ENABLES     0x20215004  // Enables auxiliary peripherals (SPI1, SPI2, mini UART)
#define AUX_MU_IO_REG   0x20215040  // Data register (read/write for mini UART)
#define AUX_MU_IER_REG  0x20215044  // Interrupt enable register
#define AUX_MU_IIR_REG  0x20215048  // Interrupt identify register
#define AUX_MU_LCR_REG  0x2021504C  // Line control register (data format settings)
#define AUX_MU_MCR_REG  0x20215050  // Modem control register (not used here)
#define AUX_MU_LSR_REG  0x20215054  // Line status register (transmit/receive state)
#define AUX_MU_MSR_REG  0x20215058  // Modem status register (not used here)
#define AUX_MU_SCRATCH  0x2021505C  // Scratch register (general-purpose storage, unused)
#define AUX_MU_CNTL_REG 0x20215060  // Mini UART control register (enables TX/RX)
#define AUX_MU_STAT_REG 0x20215064  // Mini UART status register
#define AUX_MU_BAUD_REG 0x20215068  // Baud rate register

void loader_mini_uart_init(void)
{
    unsigned int ra;

    // GPIO14  TXD0 and TXD1
    // GPIO15  RXD0 and RXD1
    // ALT function 5 for UART1
    // ALT function 0 for UART0

    // Baud rate calculation: ((250,000,000 / 115200) / 8) - 1 = 270

    // Enable the mini UART
    loader_mmio_write(AUX_ENABLES, 1);

    // Disable interrupts for the mini UART
    loader_mmio_write(AUX_MU_IER_REG, 0);

    // Disable the transmitter and receiver
    loader_mmio_write(AUX_MU_CNTL_REG, 0);

    // Configure the mini UART to 8-bit mode
    loader_mmio_write(AUX_MU_LCR_REG, 3);

    // Clear the FIFO queues and set interrupt modes
    loader_mmio_write(AUX_MU_IIR_REG, 0xC6);

    // Set baud rate to 115200
    loader_mmio_write(AUX_MU_BAUD_REG, 270);

    // Configure GPIO14 as TXD and GPIO15 as RXD (ALT5 function)
    ra = loader_mmio_read(GPFSEL1);
    ra &= ~((7 << 12) | (7 << 15));  // Clear bits 12-14 (GPIO14) and 15-17 (GPIO15)
    ra |= (2 << 12) | (2 << 15);     // Set ALT5 function for GPIO14 (TXD) and GPIO15 (RXD)
    loader_mmio_write(GPFSEL1, ra);

    // Optional: Disable pull-up/down resistors for GPIO14 and GPIO15
    loader_mmio_write(GPPUD, 0);
    loader_cpu_delay(150);
    loader_mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    loader_cpu_delay(150);
    loader_mmio_write(GPPUDCLK0, 0);

    // Enable the transmitter (TX) and receiver (RX)
    loader_mmio_write(AUX_MU_CNTL_REG, 3);
}


void loader_mini_uart_putc(uint8_t c)
{
    // Wait until the transmitter is ready
    while(1)
    {
        if(loader_mmio_read(AUX_MU_LSR_REG) & 0x20) break;
    }

    // Write the character to the data register
    loader_mmio_write(AUX_MU_IO_REG, c);
}

uint8_t loader_mini_uart_getc(void)
{
    // Wait until data is available to read
    while (1)
    {
        if ((loader_mmio_read(AUX_MU_LSR_REG) & 0x01) != 0)
            break; // Bit 0: Data ready
    }

    // Read the character from the data register
    return (unsigned char)(loader_mmio_read(AUX_MU_IO_REG) & 0xFF);
}

uint32_t loader_mini_uart_recv(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        data[i] = loader_mini_uart_getc();
    }
    return size;
}
