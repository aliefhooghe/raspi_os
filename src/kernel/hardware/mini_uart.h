#ifndef SATAN_MINI_UART_H_
#define SATAN_MINI_UART_H_

#include <stddef.h>
#include <stdint.h>


// Low level interface
void mini_uart_init(void);

uint8_t mini_uart_getc(void);
uint32_t mini_uart_recv(uint8_t *data, uint32_t size);

void mini_uart_putc(unsigned char c);
void mini_uart_wait_tx_idle(void);

// kernel log interface
#define KERNEL_ENABLE_LOG

void mini_uart_kernel_puts(const char* str);


#ifdef KERNEL_ENABLE_LOG
void mini_uart_kernel_log(const char *restrict format, ...);
#else
#define mini_uart_kernel_log(...) (void)(0, __VA_ARGS__)
#endif


#endif
