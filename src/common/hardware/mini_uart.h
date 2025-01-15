#ifndef SATAN_MINI_UART_H_
#define SATAN_MINI_UART_H_

#include <stdint.h>

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


void mini_uart_init(void);

uint8_t mini_uart_getc(void);

void mini_uart_putc(unsigned char c);
void mini_uart_puts(const char* str);
void mini_uart_put_int(uint32_t x);

#endif
