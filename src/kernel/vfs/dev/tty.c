
#include "hardware/mini_uart.h"
#include "tty.h"

//
// DRAFT: mini uart tty infrastructure
//
static int32_t _tty_mini_uart_read(struct file_handle* handle, void *data, size_t offset, size_t size)
{
    (void)handle;
    (void)offset;

    uint8_t *bdata = (uint8_t*)data;

    for (uint32_t i = 0u; i < size; i++)
    {
        const uint8_t car = mini_uart_getc();
        mini_uart_putc(car);

        if (car == '\r')
        {
            mini_uart_putc('\n');
            bdata[i] = '\n';
            return i + 1;
        }

        //
        //  TODO: place the tty infrastructure where it belong to
        //          handle backspace
        //
        // echo the car onto the tty terminal

        bdata[i] = car;
    }

    return size;
}

static int32_t _tty_mini_uart_write(struct file_handle *handle, const void *data, size_t offset, size_t size)
{
    (void)handle;
    (void)offset;

    const uint8_t *bdata = (const uint8_t*)data;

    for (uint32_t i = 0u; i < size; i++)
    {
        if (bdata[i] == '\n')
            mini_uart_putc('\r');
        mini_uart_putc(bdata[i]);
    }

    return size;
}

file_handle_t tty_init_virtual_file(void)
{
    const file_handle_t result = {
        .read = _tty_mini_uart_read,
        .write = _tty_mini_uart_write
    };
    return result;
}
