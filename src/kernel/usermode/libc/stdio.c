

#include <stddef.h>
#include <stddef.h>
#include <stdint.h>

#include "usermode/libc/stdio.h"
#include "usermode/libc/string.h"
#include "usermode/usr_syscalls.h"

int putchar(int c)
{
    const int32_t status = usr_syscall_write(1, &c, 1u);
    if (status < 0)
        return status;
    return c;
}

int puts(const char *s)
{
    const size_t len = strlen(s);
    return usr_syscall_write(1, s, len);
}

int printf(const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    return vprintf(format, ap);
    va_end(ap);
}

int vprintf(const char *restrict format, va_list ap)
{
    int escape = 0;
    char c;

    while ((c = *(format++)) != '\0') {
        if (escape) {
            switch (c) {
                // case 'u':
                //     {
                //         const uint32_t value = va_arg(ap, uint32_t);
                //         mini_uart_put_uint(value);
                //     }
                //     break;
                // case 'x':
                //     {
                //         const uint32_t value = va_arg(ap, uint32_t);
                //         mini_uart_put_uint_hex(value);
                //     }
                //     break;
                // case 'b':
                //     {
                //         const uint32_t value = va_arg(ap, uint32_t);
                //         mini_uart_put_uint_bin(value);
                //     }
                //     break;
                case 's':
                    {
                        const char *str = va_arg(ap, const char*);
                        const size_t len = strlen(str);
                        usr_syscall_write(1, str, len);
                    }
                    break;
                case 'c':
                    {
                        const char value = (char)va_arg(ap, int);
                        usr_syscall_write(1, &value, 1u);
                    }
                    break;

                case '%':
                default:
                    usr_syscall_write(1, &c, 1u);
                    break;
            }
            escape = 0;
        }
        else if (c == '%') {
            escape = 1;
        }
        else {
            usr_syscall_write(1, &c, 1u);
        }
    }

    return 0;
}

int getchar(void)
{
    uint8_t c;
    const int32_t status = usr_syscall_read(0, &c, 1u);
    if (status < 0)
        return status;
    return (int)c;
}

char *gets_s(char* data, size_t size)
{
    const int32_t status = usr_syscall_read(0, data, size - 1u);
    if (status < 0)
    {
        return NULL;
    }
    else if (data[status - 1u] == '\n')
    {
        data[status - 1u] = '\0';
    }
    else {
        data[status] = '\0';
    }
    return data;
}
