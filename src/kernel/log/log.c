
#include <stdarg.h>

#include "log.h"
#include "hardware/mini_uart.h"


void kernel_puts(const char* str)
{
    char c;
    while ((c = *(str++)) != '\0')
    {
        mini_uart_putc((unsigned char)c);
    }
}

#ifdef KERNEL_ENABLE_LOG

static void _mini_uart_put_uint(uint32_t x)
{
    char result[11] = "";
    int index = 10;

    do {
        result[--index] = '0' + (x % 10);
        x /= 10;
    } while (x > 0);

    kernel_puts(result + index);
}

static void _mini_uart_put_int(int32_t x)
{
    if (x < 0) {
        mini_uart_putc('-');
        _mini_uart_put_uint((uint32_t)(-x));
    }
    else {
        _mini_uart_put_uint((uint32_t)x);
    }
}

static void _mini_uart_put_uint_hex(uint32_t x)
{
    static const char cars[] = "0123456789abcdef";
    char result[16] = "";
    int index = 15;

    do {
        result[--index] = cars[x & 0xf];
        x >>= 4;
    } while (x > 0);

    kernel_puts(result + index);
}

static void _mini_uart_put_uint_bin(uint32_t x)
{
    char result[33] = "";
    int index = 32;

    do {
        result[--index] = '0' + (x & 0x1);
        x >>= 1;
    } while (x > 0);

    kernel_puts(result + index);
}

void kernel_log(const char *restrict format, ...)
{
    va_list ap;
    int escape = 0;
    char c;

    // use a distinct color and prefix for kernel logs
    kernel_puts("\x1b[2;34m[kernel] ");

    va_start(ap, format);
    while ((c = *(format++)) != '\0') {
        if (escape) {
            switch (c) {
                case 'u':
                    {
                        const uint32_t value = va_arg(ap, uint32_t);
                        _mini_uart_put_uint(value);
                    }
                    break;
                case 'd':
                    {
                        const int32_t value = va_arg(ap, int32_t);
                        _mini_uart_put_int(value);
                    }
                    break;
                case 'x':
                    {
                        const uint32_t value = va_arg(ap, uint32_t);
                        _mini_uart_put_uint_hex(value);
                    }
                    break;
                case 'b':
                    {
                        const uint32_t value = va_arg(ap, uint32_t);
                        _mini_uart_put_uint_bin(value);
                    }
                    break;
                case 's':
                    {
                        const char *str = va_arg(ap, const char*);
                        kernel_puts(str);
                    }
                    break;
                case 'c':
                    {
                        const char value = (char)va_arg(ap, int);
                        mini_uart_putc(value);
                    }
                    break;

                case '%':
                default:
                    mini_uart_putc('%');
                    break;
            }
            escape = 0;
        }
        else if (c == '%') {
            escape = 1;
        }
        else {
            mini_uart_putc((unsigned char)c);
        }
    }

    va_end(ap);
    kernel_puts("\x1b[0m\r\n");
}

#endif
