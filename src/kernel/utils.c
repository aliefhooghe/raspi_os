
#include "hardware/mini_uart.h"
#include "kernel.h"
#include "kernel_types.h"
#include "vfs/vfs.h"
#include "utils.h"

void load_resource_as_file(
    const char *path,
    const void *resource_data,
    const size_t resource_size)
{
    mini_uart_kernel_log(
        "load resource as file to %s (%u bytes)",
        path, resource_size);
    // TODO mkdir -p basedir

    // create file
    file_t *file = vfs_file_open(path, O_CREAT, S_IFREG);
    if (file == NULL)
        kernel_fatal_error(
            "file creation failed");

    // load resource data
    const ssize_t loaded_size = vfs_file_write(file, resource_data, resource_size);
    KERNEL_ASSERT((size_t)loaded_size == resource_size);
}
