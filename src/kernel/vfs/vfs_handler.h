#ifndef SATAN_VFS_HANDLER_H_
#define SATAN_VFS_HANDLER_H_

#include <stdint.h>
#include <stddef.h>

struct file_handle {
    int32_t (*read)(struct file_handle*, void *data, size_t offset, size_t size);
    int32_t (*write)(struct file_handle*, const void *data, size_t offset, size_t size);
};

#endif
