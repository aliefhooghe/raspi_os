#ifndef SATAN_VFS_H_
#define SATAN_VFS_H_

#include <stddef.h>
#include <stdint.h>

typedef struct file_handle file_handle_t;

typedef struct  {
    const file_handle_t *handle;
    void *fd_ctx;
} file_descriptor_t;

// vfs interface
void vfs_init(void);
// file_descriptor_t vfs_get_tty_file_descriptor(void);

// file descriptor interface: used by SYSCALLs
int32_t vfs_file_descriptor_is_null(const file_descriptor_t *fd);
file_descriptor_t vfs_file_descriptor_open(const char *path, uint32_t flags, uint32_t mode);
int32_t vfs_file_descriptor_close(file_descriptor_t *fd);
int32_t vfs_file_descriptor_read(file_descriptor_t *fd, void *data, size_t size);
int32_t vfs_file_descriptor_write(file_descriptor_t *fd, const void *data, size_t size);
int32_t vfs_file_descriptor_lseek(file_descriptor_t *fd, int32_t offset, int32_t whence);

#endif
