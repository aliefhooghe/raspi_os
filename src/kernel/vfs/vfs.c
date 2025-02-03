
#include <stdint.h>

#include "hardware/mini_uart.h"

#include "vfs/vfs.h"

#include "lib/str.h"


//
// DRAFT: mini uart tty infrastructure
//
static int32_t _tty_mini_uart_read(struct file_handle* handle, void *data, size_t offset, size_t size)
{
    (void)handle;
    (void)offset;
    return mini_uart_read(data, size);
}

static int32_t _tty_mini_uart_write(struct file_handle *handle, const void *data, size_t offset, size_t size)
{
    (void)handle;
    (void)offset;
    return mini_uart_write(data, size);
}


static file_handle_t tty_init_virtual_file(void)
{
    const file_handle_t result = {
        .read = _tty_mini_uart_read,
        .write = _tty_mini_uart_write
    };
    return result;
}

//
// file descriptor management
//
static file_descriptor_t _create_desciptor(file_handle_t *handle)
{
    const file_descriptor_t fd = {
        .handle = handle,
        .offset = 0u
    };
    return fd;
}

//
// vfs interface
//
void vfs_init(vfs_t *vfs)
{
    _memset(vfs, 0, sizeof(vfs_t));

    // init the tty as the single opened file
    vfs->file_count = 1u;
    vfs->file_table[0] = tty_init_virtual_file();
}

file_descriptor_t vfs_get_tty_file_descriptor(vfs_t *vfs)
{
    return _create_desciptor(&vfs->file_table[0u]);
}

//
//  File Descriptor interface
//

int32_t file_descriptor_read(file_descriptor_t *fd, void *data, size_t size)
{
    const int32_t status = fd->handle->read(fd->handle, data, fd->offset, size);
    if (status < 0)
        return status;
    fd->offset += status;
    return status;
}

int32_t file_descriptor_write(file_descriptor_t *fd, const void *data, size_t size)
{
    const int32_t status = fd->handle->write(fd->handle, data, fd->offset, size);
    if (status < 0)
        return status;
    fd->offset += status;
    return status;
}
