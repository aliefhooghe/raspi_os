#ifndef SATAN_VFS_DEV_SDCARD_H_
#define SATAN_VFS_DEV_SDCARD_H_

#include "vfs/device_ops.h"
//
// Expose sdcard as a block device
//
int create_sdcard_disk(block_device_t *device);

#endif
