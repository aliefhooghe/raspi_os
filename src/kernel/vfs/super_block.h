#ifndef SATAN_KERNEL_VFS_SUPER_BLOCK_H_
#define SATAN_KERNEL_VFS_SUPER_BLOCK_H_

#include "inode.h"

struct super_block {
    // dev_t device;
    //  Root Inode:
    //  - have a special number known by the fs
    //  - must be loaded and read at startup  (super_bloc->alloc + super_bloc->read)
    // inode_t *root_node;
    //

    ino_t root_ino;
    const super_block_ops_t *ops;
    void *private;
};

struct super_block_ops {

    // inode allocation
    inode_t *(*alloc_inode) (super_block_t*);
    void (*free_inode) (super_block_t*, inode_t *inode);

    // read/write inode on backend
    int (*read_inode) (super_block_t *sb, ino_t ino, inode_t *inode);
    int (*write_inode) (super_block_t *sb, inode_t *inode);
    
};

#endif
