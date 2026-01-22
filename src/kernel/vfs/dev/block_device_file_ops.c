
#include "block_device_file_ops.h"
#include "kernel.h"
#include "kernel_types.h"
#include "lib/str.h"
#include "utils.h"
#include "vfs/device_ops.h"
#include "vfs/inode.h"
#include <stdint.h>

// assertion on block size
#define BLOCK_SIZE (512u)

static ssize_t _block_device_read(
    file_t *file, void *data, size_t size)
{
    uint8_t *const dest = (uint8_t*)data;

    // TODO: we could avoid transfering data with this buffer when size > 512
    uint8_t block[BLOCK_SIZE];
    block_device_t *device = (block_device_t*)file->inode->private;

    KERNEL_ASSERT(BLOCK_SIZE == device->block_size);
    size_t total_read_size = 0u;
    const off_t offset = file->pos;
    off_t block_index = offset >> 9; // / 512
    off_t block_offset = offset & 0x1FFu; // % 512

    while (size > 0) {
        ssize_t status = device->ops->read_block(
            device->private, block_index, block);
        if (status < 0) {
            // if nothing was read: error
            if (total_read_size == 0) {
                return -1;
            }
            else {
                // TODO: if out of bound: OK.
                // but in case of device error, we should throw an error !!
                break;
            }
        }

        const size_t read_size = size_t_min(
            BLOCK_SIZE - block_offset,
            size
        );
        _memcpy(
            dest + total_read_size, block + block_offset, read_size);

        size -= read_size;
        total_read_size += read_size;

        block_offset = 0u;
        block_index++;
    }

    return total_read_size;
}

static ssize_t _block_device_write(
    file_t *file, const void *data, size_t size)
{
    (void)file;
    (void)data;
    (void)size;
    // block_device_t *device = (block_device_t*)file->inode->private;
    kernel_fatal_error("block device file ops write is not implemented");
    // TODO: implement write
    return -1;
}

static ssize_t _block_device_seek(
    file_t *file, int32_t offset, int32_t whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    // block_device_t *device = (block_device_t*)file->inode->private;
    kernel_fatal_error("block device file ops seek is not implemented");
    // TODO: implement seek
    return -1;
}

const file_ops_t block_device_file_ops = {
    .read = _block_device_read,
    .write = _block_device_write,
    .seek = _block_device_seek,
    .readdir = NULL
};
