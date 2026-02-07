
#include <stdint.h>

#include "hardware/mini_uart.h"
#include "hardware/sd_host/sd_host.h"
#include "kernel.h"

#include "sdcard.h"

static int _sdcard_disk_read_block(void *private, uint32_t index, void *block)
{
    mini_uart_kernel_log("sdcard-block-device: read block @ %u", index);
    (void)private;
    KERNEL_ASSERT(0 == sdhost_read_block(index, block));
    return 1;
}

static int _sdcard_disk_write_block(void *private, uint32_t index, const void *block)
{
    (void)private;
    (void)index;
    (void)block;
    kernel_fatal_error("sdcard-block-device: write is not implemented");
    return 1;
}

static const block_device_ops_t _sdcard_disk_ops = {
    .read_block = _sdcard_disk_read_block,
    .write_block = _sdcard_disk_write_block
};

int create_sdcard_disk(block_device_t *device)
{
    device->ops = &_sdcard_disk_ops;
    device->block_size = 512u;
    device->private = NULL;
    return 0;
}
