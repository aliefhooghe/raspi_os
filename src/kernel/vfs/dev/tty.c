
#include "memory/memory_allocator.h"
#include "hardware/mini_uart.h"
#include "vfs/device_ops.h"
#include "vfs/inode.h"
#include "tty.h"

//
// mini uart tty character device driver
//

static ssize_t _tty_mini_uart_read(
    file_t *file, void *data, size_t size)
{
    (void)file;
    
    uint8_t *car_buf = (uint8_t*)data;

    for (uint32_t i = 0u; i < size; i++)
    {
        const uint8_t car = mini_uart_getc();

        // echo the car onto the tty terminal
        mini_uart_putc(car);

        if (car == '\r')
        {
            mini_uart_putc('\n');
            car_buf[i] = '\n';
            return i + 1;
        }
        car_buf[i] = car;
    }

    return size;
}

static ssize_t _tty_mini_uart_write(
    file_t *file, const void *data, size_t size)
{
    (void)file;

    const uint8_t *car_buf = (const uint8_t*)data;

    for (uint32_t i = 0u; i < size; i++)
    {
        if (car_buf[i] == '\n')
            mini_uart_putc('\r');
        mini_uart_putc(car_buf[i]);
    }

    return size;
}

const character_device_ops_t dev_tty_ops = {
    .read = _tty_mini_uart_read,
    .write = _tty_mini_uart_write,
    .seek = NULL,
    .readdir = NULL
};


