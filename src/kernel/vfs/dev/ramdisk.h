#ifndef SATAN_VFS_DEV_RAMDISK_H_
#define SATAN_VFS_DEV_RAMDISK_H_

#include "vfs/device_ops.h"

//
// Expose a ram section as a block device
// 

int create_ramdisk(block_device_t *device, void *mem, size_t size);
int create_section_ramdisk(block_device_t *device);
// void section_ramdisk_free(block_device_t *device);



#endif

