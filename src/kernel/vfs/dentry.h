#ifndef SATAN_KERNEL_VFS_DENTRY_H_
#define SATAN_KERNEL_VFS_DENTRY_H_

#include <stdint.h>
#include "inode.h"


#define DENTRY_MAX_CHILREN_COUNT (8u)
#define DENTRY_MAX_NAME_LEN (16u)


typedef struct dentry dentry_t;

struct dentry {
    char name[DENTRY_MAX_NAME_LEN];
    // uint8_t type; : in inode
    inode_t *inode;
    dentry_t *parent;
    dentry_t *children[DENTRY_MAX_CHILREN_COUNT];
    size_t child_count;
};

#endif
