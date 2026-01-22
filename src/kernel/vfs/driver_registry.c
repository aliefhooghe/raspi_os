
#include "driver_registry.h"
#include "dev/tty.h"
#include "hardware/mini_uart.h"
#include "kernel.h"
#include "kernel_types.h"
#include "vfs/dev/ramdisk.h"
#include "vfs/device_ops.h"
#include <stdint.h>


//
// Character device registry
#define CHAR_DEV_MAJOR_COUNT (1u)
#define DEV_TTY_MAJOR (0u)
#define DEV_TTY_MINI_UART_MINOR (0u)

static char_device_t _char_devices[CHAR_DEV_MAJOR_COUNT][1] = {
    [DEV_TTY_MAJOR] = {
        [DEV_TTY_MINI_UART_MINOR] = {
            .ops = &dev_tty_ops,
            .private = NULL
        }
    } 
};

//
// Block device registry
#define BLOCK_DEV_MAJOR_COUNT (1u)
#define DEV_RAMDISK_MAJOR (0u)
#define DEV_RAMDISK_MINOR (0u)

static block_device_t _block_devices[BLOCK_DEV_MAJOR_COUNT][1];

void driver_registry_init(void)
{
    mini_uart_kernel_log("[driver registry] initialize ramdisk");
    create_section_ramdisk(
        &_block_devices[DEV_RAMDISK_MAJOR][DEV_RAMDISK_MINOR]);
}

char_device_t *get_char_device(dev_t dev)
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
    KERNEL_ASSERT(minor == DEV_TTY_MINI_UART_MINOR);

    return &_char_devices[major][minor];
}

block_device_t *get_block_device(dev_t dev)
{
    const uint16_t major = DEV_MAJOR(dev);
    const uint16_t minor = DEV_MINOR(dev);

    // check major is in valid range
    if (major >= BLOCK_DEV_MAJOR_COUNT) {
        mini_uart_kernel_log(
            "[driver registry] out of range block major number",
            major);
        return NULL;
    }

    // for now: only one minor per driver
    KERNEL_ASSERT(minor == DEV_TTY_MINI_UART_MINOR);

    return &_block_devices[major][minor];
}
