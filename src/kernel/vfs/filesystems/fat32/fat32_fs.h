#ifndef SATAN_VFS_FAT32_FS_H_
#define SATAN_VFS_FAT32_FS_H_

#include "vfs/device_ops.h"
#include "vfs/inode.h"

super_block_t *fat32_create_filesystem(block_device_t *blk_dev);

#endif
