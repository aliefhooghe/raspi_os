#ifndef SATAN_VFS_DEVICE_OPERATIONS_H_
#define SATAN_VFS_DEVICE_OPERATIONS_H_

#include "inode.h"

//
// driver interface
typedef struct block_device_ops block_device_ops_t;
typedef file_ops_t character_device_ops_t;

struct block_device_ops {
    int (*read_block)(void *private, uint64_t index, void *block);
    int (*write_block)(void *private, uint64_t index, const void *block);
    int (*sync)(void *private);
};


#endif
