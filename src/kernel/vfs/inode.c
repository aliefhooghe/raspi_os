
#include "kernel.h"
#include "memory/memory_allocator.h"
#include "inode.h"



file_t *default_file_open(inode_t *inode)
{
    KERNEL_ASSERT(inode != NULL);
    file_t *file = memory_calloc(sizeof(file_t));
    KERNEL_ASSERT(file != NULL);
    file->inode = inode;
    return file;
}

int default_file_release(inode_t *inode, file_t *file)
{
    KERNEL_ASSERT(inode != NULL);
    KERNEL_ASSERT(file != NULL);
    KERNEL_ASSERT(file->fd_count == 0u);
    memory_free(file);
    return 0;
}
