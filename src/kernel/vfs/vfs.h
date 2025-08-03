#ifndef SATAN_VFS_H_
#define SATAN_VFS_H_

#include <stddef.h>
#include <stdint.h>

#include "kernel_types.h"

//
// VFS Public Interface
//
typedef struct file file_t;

void vfs_init(void);

int32_t vfs_mknod(const char *path, mode_t mode, dev_t dev);
int32_t vfs_mkdir(const char *path, mode_t mode);

file_t *vfs_file_open(const char *path, uint32_t flags, mode_t mode);
int32_t vfs_file_close(file_t *file);

ssize_t vfs_file_read(file_t *file, void *data, size_t size);
ssize_t vfs_file_readdir(file_t *file, dirent *entries, size_t count);
ssize_t vfs_file_write(file_t *file, const void *data, size_t size);
ssize_t vfs_file_lseek(file_t *file, int32_t offset, int32_t whence);


// int32_t usr_syscall_mount(const char *dev, const char *target, const char *fstype);
// int32_t usr_syscall_mknod(const char *path, mode_t mode, dev_t dev);

#endif
