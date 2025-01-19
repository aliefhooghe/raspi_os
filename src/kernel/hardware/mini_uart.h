#ifndef SATAN_MINI_UART_H_
#define SATAN_MINI_UART_H_

#include <stdint.h>


void mini_uart_init(void);

uint8_t mini_uart_getc(void);
uint32_t mini_uart_recv(uint8_t *data, uint32_t size);

void mini_uart_putc(unsigned char c);
void mini_uart_wait_tx_idle(void);
void mini_uart_puts(const char* str);
void mini_uart_put_int(uint32_t x);
void mini_uart_put_hex(uint32_t x);
void mini_uart_put_bin(uint32_t x);

#endif
