#ifndef SATAN_SERIAL_LOADER_LIB_H_
#define SATAN_SERIAL_LOADER_LIB_H_

/**
 * A minimal tiny library for the serial bootloader. The mini uart is used
 * by polling the rx register in order to avoid using interupts here.
 */

#include <stdint.h>
#include <stddef.h>

// utils
extern void loader_cpu_delay(uint32_t cycle_count);
void *loader_memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size);

// mini uart
void loader_mini_uart_init(void);
void loader_mini_uart_putc(uint8_t c);
uint8_t loader_mini_uart_getc(void);
uint32_t loader_mini_uart_recv(uint8_t *data, uint32_t size);

#endif
