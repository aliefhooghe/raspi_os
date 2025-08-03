#ifndef SATAN_VFS_DEV_TTY_H_
#define SATAN_VFS_DEV_TTY_H_

#include "vfs/device_ops.h"

const character_device_ops_t *dev_tty_create(void);

#endif

