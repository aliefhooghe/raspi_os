#include <stdint.h>
#include <stdarg.h>

#include "hardware/gpio.h"
#include "hardware/mmio.h"
#include "hardware/io_registers.h"
#include "hardware/mini_uart.h"

void mini_uart_init(void)
{
    // GPIO14  TXD0 and TXD1
    // GPIO15  RXD0 and RXD1
    // ALT function 5 for UART1
    // ALT function 0 for UART0


    // Enable the mini UART
    mmio_write(REG__AUX_ENABLES, AUX_MINI_UART);

    // Disable interrupts
    mmio_write(REG__AUX_MU_IER, 0x0);

    // Disable the transmitter and receiver
    mmio_write(REG__AUX_MU_CNTL, 0);

    // Configure the mini UART to 8-bit mode
    mmio_write(REG__AUX_MU_LCR, AUX_MU_LCR_DATA_8BIT);

    // Clear the FIFO queues and set interrupt modes
    mmio_write(REG__AUX_MU_IIR, 0xC6);

    // Set baud rate to 115200
    // Baud rate calculation: ((250,000,000 / 115200) / 8) - 1 = 270
    mmio_write(REG__AUX_MU_BAUD, 270);

    // Configure GPIO14 as TXD and GPIO15 as RXD (ALT5 function)
    gpio_select_function(14, GPIO_F_ALT_5);
    gpio_select_function(15, GPIO_F_ALT_5);

    // Optional: Disable pull-up/down resistors for GPIO14 and GPIO15
    gpio_set_pin_mode(14, GPIO_MODE_PULL_FLOATING);
    gpio_set_pin_mode(15, GPIO_MODE_PULL_FLOATING);

    // Enable the transmitter (TX) and receiver (RX)
    mmio_write(
        REG__AUX_MU_CNTL,
        AUX_MU_CNTL_RX_EN | AUX_MU_CNTL_TX_EN);
}


void mini_uart_putc(unsigned char c)
{
    // Wait until the transmitter is ready
    while(1)
    {
        if(mmio_read(REG__AUX_MU_LSR) & 0x20) break;
        asm volatile ("wfe");
    }

    // Write the character to the data register
    mmio_write(REG__AUX_MU_IO, c);
}

void mini_uart_wait_tx_idle(void)
{
    // Wait until the transmitter is idle
    while(1)
    {
        if(mmio_read(REG__AUX_MU_LSR) & 0x40) break;
        asm volatile ("wfe");
    }
}

uint8_t mini_uart_getc(void)
{
    // Wait until data is available to read
    while (1)
    {
        if ((mmio_read(REG__AUX_MU_LSR) & 0x01) != 0) {
            break; // Bit 0: Data ready
        }
        asm volatile ("wfe");
    }

    // Read the character from the data register
    return (unsigned char)(mmio_read(REG__AUX_MU_IO) & 0xFF);
}

uint32_t mini_uart_recv(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        data[i] = mini_uart_getc();
    }
    return size;
}
