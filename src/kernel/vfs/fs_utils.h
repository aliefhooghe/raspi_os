#ifndef SATAN_VFS_FS_UTILS_H_
#define SATAN_VFS_FS_UTILS_H_

#include <stdint.h>

#include "vfs/inode.h"
#include "kernel_types.h"

//
// Get an absolute offset based on a file and a seek whence
off_t get_seek_ref_offset(file_t *file, int32_t whence);

#endif
