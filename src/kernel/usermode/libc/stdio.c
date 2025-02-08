

#include <stddef.h>
#include <stddef.h>
#include <stdint.h>

#include "usermode/libc/stdio.h"
#include "memory/allocator.h"
#include "usermode/libc/string.h"
#include "lib/str.h"
#include "usermode/usr_syscalls.h"


// void mini_uart_put_uint_hex(uint32_t x)
// {
//     static const char cars[] = "0123456789abcdef";
//     char result[9] = "";
//     int index = 8;

//     do {
//         result[--index] = cars[x & 0xf];
//         x >>= 4;
//     } while (x > 0);

//     mini_uart_puts(result + index);
// }

// void mini_uart_put_uint_bin(uint32_t x)
// {
//     char result[33] = "";
//     int index = 32;

//     do {
//         result[--index] = '0' + (x & 0x1);
//         x >>= 1;
//     } while (x > 0);

//     mini_uart_puts(result + index);
// }

//
//  STDIO initialization stub
//
FILE *get_stdout(void)
{
    FILE *fd = mem_alloc(sizeof(FILE));
    fd->fd = 1;
    fd->write_buffer_cursor = 0u;
    return fd;
}

//
//  files manipulation
//
int fflush(FILE *stream)
{
    if (stream->write_buffer_cursor > 0)
    {
        const int status = usr_syscall_write(
            stream->fd, stream->write_buffer, stream->write_buffer_cursor);
        if (status < 0)
        {
            return -1;
        }
        else
        {
            stream->write_buffer_cursor = 0u;
            return 0;
        }
    }
    return 0;
}

size_t fread(void *restrict ptr, size_t size, size_t n, FILE *restrict stream)
{
    const size_t data_size = size * n;
    const int32_t status = usr_syscall_read(stream->fd, ptr, data_size);
    if (status < 0)
    {
        return 0;
    }
    else
    {
        return status;
    }
}

size_t fwrite(const void *restrict ptr, size_t size, size_t n, FILE *restrict stream)
{
    const uint8_t *data_ptr = (const uint8_t*)ptr;
    const size_t data_size = size * n;
    size_t total_size = data_size;

    while (total_size > 0u) {

        const size_t remaining_buffer_size =
            (WRITE_BUFFER_SIZE - stream->write_buffer_cursor);

        if (remaining_buffer_size == 0u)
        {
            fflush(stream);
            continue;
        }

        const size_t chunk_size =
            (total_size > remaining_buffer_size) ?
                remaining_buffer_size: total_size;

        _memcpy(
            stream->write_buffer + stream->write_buffer_cursor,
            data_ptr, chunk_size);
        stream->write_buffer_cursor += chunk_size;

        total_size -= chunk_size;
        data_ptr += chunk_size;
    }

    return total_size;
}


int fputs(const char *restrict s, FILE *restrict stream)
{
    const size_t len = strlen(s);
    return fwrite(s, len, 1, stream);
}

static void _fput_uint(FILE *restrict stream, uint32_t x)
{
    char result[11] = {0};
    int index = 10;

    do {
        result[--index] = '0' + (x % 10);
        x /= 10;
    } while (x > 0);

    fwrite(result + index, 11 - index, 1, stream);
}


// int putchar(int c)
// {
//     const int32_t status = usr_syscall_write(1, &c, 1u);
//     if (status < 0)
//         return status;
//     return c;
// }

// int puts(const char *s)
// {
//     const size_t len = strlen(s);
//     return usr_syscall_write(1, s, len);
// }

// int snprintf(char *restrict str, size_t size, const char *restrict format, ...)
// {
//     va_list ap;
//     va_start(ap, format);
//     const int status = vsnprintf(str, size, format, ap);
//     va_end(ap);
//     return status;
// }

// int vsnprintf(char *restrict str, size_t size, const char *restrict format, va_list ap)
// {

// }

int fprintf(FILE *restrict stream, const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    const int status = vfprintf(stream, format, ap);
    va_end(ap);
    return status;
}

int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap)
{
    int escape = 0;
    char c;

    while ((c = *(format++)) != '\0') {
        if (escape) {
            switch (c) {
                case 'u':
                    {
                        const uint32_t value = va_arg(ap, uint32_t);
                        _fput_uint(stream, value);
                    }
                    break;
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
                        fwrite(str, len, 1u, stream);
                    }
                    break;
                case 'c':
                    {
                        const char value = (char)va_arg(ap, int);
                        fwrite(&value, 1u, 1u, stream);
                    }
                    break;

                case '%':
                default:
                    fwrite(&c, 1u, 1u, stream);
                    break;
            }
            escape = 0;
        }
        else if (c == '%') {
            escape = 1;
        }
        else {
            fwrite(&c, 1u, 1u, stream);
        }
    }

    fflush(stream);
    return 0;
}

// ---

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
