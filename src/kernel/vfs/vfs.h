#ifndef SATAN_VFS_H_
#define SATAN_VFS_H_

#include <stddef.h>
#include <stdint.h>

typedef struct file_handle {
    int32_t (*read)(struct file_handle*, void *data, size_t offset, size_t size);
    int32_t (*write)(struct file_handle*, const void *data, size_t offset, size_t size);
} file_handle_t;


typedef struct  {
    file_handle_t *handle;
    uint32_t offset;

    // todo: add r/w mode here
} file_descriptor_t;

typedef struct {
    file_handle_t file_table[1];
    uint32_t file_count;
} vfs_t;

// vfs interface
void vfs_init(void);

// draft:
file_descriptor_t vfs_get_tty_file_descriptor(void);

// file descriptor interface
int32_t file_descriptor_read(file_descriptor_t *fd, void *data, size_t size);
int32_t file_descriptor_write(file_descriptor_t *fd, const void *data, size_t size);

#endif
