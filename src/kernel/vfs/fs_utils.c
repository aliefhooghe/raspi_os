
#include "kernel.h"
#include "fs_utils.h"


off_t get_seek_ref_offset(file_t *file, int32_t whence)
{
    switch (whence) {
        case SEEK_SET: return 0u; break;
        case SEEK_END: return file->inode->size; break;
        case SEEK_CUR: return file->pos; break;
        default:
            kernel_fatal_error("seek: invalid whence");
            return -1;
    }
}

