#ifndef SATAN_VFS_DRIVER_REGISTRY_H_
#define SATAN_VFS_DRIVER_REGISTRY_H_

#include "device_ops.h"
#include "kernel_types.h"

// const character_device_ops_t *load_char_device(void);
//

const char_device_t *get_character_device(dev_t dev);

#endif
