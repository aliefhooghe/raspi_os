#ifndef SATAN_MINI_UART_H_
#define SATAN_MINI_UART_H_

#include <stddef.h>
#include <stdint.h>

// Mask for auxiliary registers (IRQ/Enable)
#define AUX_MINI_UART 0x1u
#define AUX_SPI1      0x2u
#define AUX_SPI2      0x4u

// see REG__AUX_MU_IER
#define AUX_MU_IER_EN_TX_INT 0x1u
#define AUX_MU_IER_EN_RX_INT 0x2u

// see REG__AUX_MU_LCR
#define AUX_MU_LCR_DATA_8BIT 0x1u

// see REG__AUX_MU_CNTL
#define AUX_MU_CNTL_RX_EN 0x1u
#define AUX_MU_CNTL_TX_EN 0x1u

// Low level interface
void mini_uart_init(void);

uint8_t mini_uart_getc(void);
uint32_t mini_uart_recv(uint8_t *data, uint32_t size);

void mini_uart_putc(unsigned char c);
void mini_uart_wait_tx_idle(void);

#endif
