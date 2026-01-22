#ifndef SATAN_VFS_DRIVER_REGISTRY_H_
#define SATAN_VFS_DRIVER_REGISTRY_H_

#include "device_ops.h"
#include "kernel_types.h"


void driver_registry_init(void);

char_device_t *get_char_device(dev_t dev);
block_device_t *get_block_device(dev_t dev);

#endif
