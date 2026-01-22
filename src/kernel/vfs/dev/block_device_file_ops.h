#ifndef SATAN_VFS_DEV_BLK_DEV_FILE_OPS_H_
#define SATAN_VFS_DEV_BLK_DEV_FILE_OPS_H_

#include "vfs/inode.h"

//
// File ops adapter on a block device.
extern const file_ops_t block_device_file_ops;

#endif
