#ifndef SATAN_VFS_DRIVER_REGISTRY_H_
#define SATAN_VFS_DRIVER_REGISTRY_H_

#include "device_ops.h"

const character_device_ops_t *load_char_device(void);

#endif
