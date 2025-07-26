
#include "vfs/vfs.h"
#include "vfs/vfs_handler.h"
#include "vfs/dev/tty.h"
#include "lib/str.h"


/**
 *  vfs
 */
typedef struct {
    file_handle_t file_table[1];
    uint32_t file_count;
} vfs_t;


/**
 *  global vfs
 */
static vfs_t _vfs;

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
void vfs_init(void)
{
    _memset(&_vfs, 0, sizeof(vfs_t));

    // init the tty as the single opened file
    _vfs.file_count = 1u;
    _vfs.file_table[0] = tty_init_virtual_file();
}

file_descriptor_t vfs_get_tty_file_descriptor(void)
{
    return _create_desciptor(&_vfs.file_table[0u]);
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
