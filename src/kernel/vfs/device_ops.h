#ifndef SATAN_VFS_DEVICE_OPERATIONS_H_
#define SATAN_VFS_DEVICE_OPERATIONS_H_

#include <stddef.h>

#include "inode.h"

//
// driver interfaces
//

//
// character device interface

// TODO: character device should maybe be separated from file ops.
typedef file_ops_t character_device_ops_t;

typedef struct char_device {
    const character_device_ops_t *ops;
    void *private;
} char_device_t;

//
// block device interface
typedef struct block_device_ops block_device_ops_t;

struct block_device_ops {
    // seems bad: shouuld return 0
    // return the number of block read on sucess (1)
    int (*read_block)(void *private, uint64_t index, void *block);

    // return the number of block written if success (1)
    int (*write_block)(void *private, uint64_t index, const void *block);
};

typedef struct block_device {
    const block_device_ops_t *ops;
    size_t block_size;
    void *private;
} block_device_t;


#endif
