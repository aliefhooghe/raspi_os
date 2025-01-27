#include <stdint.h>

#include "hardware/mmio.h"
#include "hardware/cpu.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"

void mini_uart_init(void)
{
    unsigned int ra;

    // GPIO14  TXD0 and TXD1
    // GPIO15  RXD0 and RXD1
    // ALT function 5 for UART1
    // ALT function 0 for UART0

    // Baud rate calculation: ((250,000,000 / 115200) / 8) - 1 = 270

    // Enable the mini UART
    mmio_write(REG__AUX_ENABLES, 1);

    // Enable rx interrupts
    mmio_write(REG__AUX_MU_IER_REG, 0x5);

    // Disable the transmitter and receiver
    mmio_write(REG__AUX_MU_CNTL_REG, 0);

    // Configure the mini UART to 8-bit mode
    mmio_write(REG__AUX_MU_LCR_REG, 3);

    // Clear the FIFO queues and set interrupt modes
    mmio_write(REG__AUX_MU_IIR_REG, 0xC6);

    // Set baud rate to 115200
    mmio_write(REG__AUX_MU_BAUD_REG, 270);

    // Configure GPIO14 as TXD and GPIO15 as RXD (ALT5 function)
    ra = mmio_read(REG__GPFSEL1);
    ra &= ~((7 << 12) | (7 << 15));  // Clear bits 12-14 (GPIO14) and 15-17 (GPIO15)
    ra |= (2 << 12) | (2 << 15);     // Set ALT5 function for GPIO14 (TXD) and GPIO15 (RXD)
    mmio_write(REG__GPFSEL1, ra);

    // Optional: Disable pull-up/down resistors for GPIO14 and GPIO15
    mmio_write(REG__GPPUD, 0);
    cpu_delay(150);
    mmio_write(REG__GPPUDCLK0, (1 << 14) | (1 << 15));
    cpu_delay(150);
    mmio_write(REG__GPPUDCLK0, 0);

    // Enable the transmitter (TX) and receiver (RX)
    mmio_write(REG__AUX_MU_CNTL_REG, 3);
}


void mini_uart_putc(unsigned char c)
{
    // Wait until the transmitter is ready
    while(1)
    {
        if(mmio_read(REG__AUX_MU_LSR_REG) & 0x20) break;
    }

    // Write the character to the data register
    mmio_write(REG__AUX_MU_IO_REG, c);
}

void mini_uart_wait_tx_idle(void)
{
    // Wait until the transmitter is idle
    while(1)
    {
        if(mmio_read(REG__AUX_MU_LSR_REG) & 0x40) break;
    }
}

uint8_t mini_uart_getc(void)
{
    // Wait until data is available to read
    while (1)
    {
        if ((mmio_read(REG__AUX_MU_LSR_REG) & 0x01) != 0)
            break; // Bit 0: Data ready
    }

    // Read the character from the data register
    return (unsigned char)(mmio_read(REG__AUX_MU_IO_REG) & 0xFF);
}

uint32_t mini_uart_recv(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        data[i] = mini_uart_getc();
    }
    return size;
}

void mini_uart_puts(const char* str)
{
    for (uint32_t i = 0; str[i] != '\0'; i ++)
        mini_uart_putc((unsigned char)str[i]);
}

void mini_uart_put_int(uint32_t x)
{
    char result[11] = "";
    int index = 10;

    do {
        result[--index] = '0' + (x % 10);
        x /= 10;
    } while (x > 0);

    mini_uart_puts(result + index);
}

void mini_uart_put_hex(uint32_t x)
{
    static const char cars[] = "0123456789abcdef";
    char result[9] = "";
    int index = 8;

    do {
        result[--index] = cars[x & 0xf];
        x >>= 4;
    } while (x > 0);

    mini_uart_puts(result + index);
}

void mini_uart_put_bin(uint32_t x)
{
    char result[33] = "";
    int index = 32;

    do {
        result[--index] = '0' + (x & 0x1);
        x >>= 1;
    } while (x > 0);

    mini_uart_puts(result + index);
}
