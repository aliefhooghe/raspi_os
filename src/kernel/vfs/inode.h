#ifndef SATAN_KERNEL_VFS_INODE_H_
#define SATAN_KERNEL_VFS_INODE_H_

#include "kernel_types.h"
#include <stddef.h>
#include <stdint.h>

typedef struct file file_t;
typedef struct file_ops file_ops_t;
typedef struct inode_ops inode_ops_t;
typedef struct inode inode_t;
typedef uint32_t ino_t;
typedef struct super_block super_block_t;
typedef struct super_block_ops super_block_ops_t;

//
//  Open file structure
struct file {
    size_t fd_count;     // fd  reference count
    inode_t *inode;
    off_t pos;           // current offset
    void *private;
};

struct file_ops {
    file_t *(*open) (inode_t *inode);
    int (*release) (inode_t *inode, file_t *file);
    ssize_t (*read) (file_t *file, void *data, size_t size);
    ssize_t (*write) (file_t *file, const void *data, size_t size);
    ssize_t (*seek) (file_t *file, int32_t offset, int32_t whence);
    int (*readdir)(file_t *file, struct dirent *entries, size_t count);
};

//
// Inode
struct inode_ops {    
    inode_t *(*lookup)(inode_t *dir, const char *name);
    inode_t *(*create)(inode_t *dir, const char *name, mode_t mode);
    inode_t *(*mkdir)(inode_t *dir, const char *name, mode_t mode);
    inode_t *(*mknod)(inode_t *dir, const char *name, mode_t mode, dev_t dev);

    int (*link)(inode_t *src, inode_t *dir, const char *new_name);
    int (*unlink)(inode_t *dir, const char *name);
    int (*rmdir)(inode_t *dir, const char *name);
};

struct inode {
    ino_t ino;
    dev_t device;
    loff_t size;
    mode_t mode;
    size_t link_count;           // TODO: use number of references to this inode
    const file_ops_t * file_ops; 
    const inode_ops_t * inode_ops;
    super_block_t *super_block;
    void * private;
};

//
// Default implementations for file open / release
file_t *default_file_open(inode_t *inode);
int default_file_release(inode_t *inode, file_t *file);

#endif
