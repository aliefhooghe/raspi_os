
#include "driver_registry.h"
#include "dev/tty.h"
#include "hardware/mini_uart.h"
#include "kernel.h"
#include "kernel_types.h"
#include "vfs/device_ops.h"
#include <stdint.h>

// #define BLOCK_DEVICE_COUNT (1u)
// #define CHAR_DEVICE_COUNT (2u)


#define DEV_TTY_MAJOR (0u)
#define DEV_TTY_MINI_UART_MINOR (0u)

#define CHAR_DEV_MAJOR_COUNT (1u)


// static const block_device_t _block_devices[1];
static const char_device_t _char_devices[1][1] = {
    [DEV_TTY_MAJOR] = {
        [DEV_TTY_MINI_UART_MINOR] = {
            .ops = &dev_tty_ops,
            .private = NULL
        }
    } 
};

const char_device_t *get_character_device(dev_t dev)
{
    const uint16_t major = DEV_MAJOR(dev);
    const uint16_t minor = DEV_MINOR(dev);

    // check major is in valid range
    if (major >= CHAR_DEV_MAJOR_COUNT) {
        mini_uart_kernel_log(
            "[driver registry] out of range char major number",
            major);
        return NULL;
    }

    // for now: only one minor per driver
    KERNEL_ASSERT(minor == 0u);

    return &_char_devices[major][minor];
}
