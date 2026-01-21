
#include "memory/memory_allocator.h"
#include "hardware/mini_uart.h"
#include "vfs/device_ops.h"
#include "vfs/inode.h"
#include "tty.h"

//
// mini uart tty character device driver
//

static ssize_t _tty_mini_uart_read(
    file_t *file, void *data, size_t size, off_t *offset)
{
    (void)file;
    (void)offset;
    
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
    file_t *file, const void *data, size_t size, off_t *offset)
{
    (void)file;
    (void)offset;

    const uint8_t *car_buf = (const uint8_t*)data;

    for (uint32_t i = 0u; i < size; i++)
    {
        if (car_buf[i] == '\n')
            mini_uart_putc('\r');
        mini_uart_putc(car_buf[i]);
    }

    return size;
}

static file_t *_tty_mini_uart_open(inode_t *inode)
{
    // TODO: a default implem for that one and some others ??
    file_t *file = memory_calloc(sizeof(file_t));
    file->inode = inode;
    file->pos = 0u;
    // file->private = NULL;
    return file; 
}

const character_device_ops_t dev_tty_ops = {
    .read = _tty_mini_uart_read,
    .write = _tty_mini_uart_write,
    .seek = NULL,
    .readdir = NULL,
    .open = _tty_mini_uart_open,
    .release = NULL,
};


