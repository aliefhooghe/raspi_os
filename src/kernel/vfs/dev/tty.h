#ifndef SATAN_VFS_DEV_TTY_H_
#define SATAN_VFS_DEV_TTY_H_

#include "vfs/device_ops.h"


// do we need a character_device_t ? (probably yes to hold private data)
// const character_device_ops_t *dev_tty_create(void);

extern const character_device_ops_t dev_tty_ops;

#endif

