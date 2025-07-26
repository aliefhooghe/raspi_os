#ifndef SATAN_VFS_HANDLER_H_
#define SATAN_VFS_HANDLER_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int32_t (*read)(void *backend, void *ctx, void *data, size_t size);
    int32_t (*write)(void *backend, void *ctx, const void *data, size_t size);
} file_ops_t;

struct file_handle {
    void *backend;
    file_ops_t ops;
};

#endif
