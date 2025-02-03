#ifndef SATAN_MINI_UART_H_
#define SATAN_MINI_UART_H_

#include <stddef.h>
#include <stdint.h>


void mini_uart_init(void);

uint8_t mini_uart_getc(void);
uint32_t mini_uart_recv(uint8_t *data, uint32_t size);

void mini_uart_putc(unsigned char c);
void mini_uart_wait_tx_idle(void);

// print interface
void mini_uart_puts(const char* str);
void mini_uart_printf(const char *restrict format, ...);

void mini_uart_put_uint(uint32_t x);
void mini_uart_put_uint_hex(uint32_t x);
void mini_uart_put_uint_bin(uint32_t x);

#endif
