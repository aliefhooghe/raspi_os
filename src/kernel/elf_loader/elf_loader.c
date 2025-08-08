
#include "kernel.h"
#include "vfs/vfs.h"
#include "elf_format.h"



static void _test(const char *path)
{
    file_t *file = vfs_file_open(path, 0u, 0u);
    if (file == NULL)
        kernel_fatal_error("could not open elf file");


    elf32_header_t header;
    const ssize_t status = vfs_file_read(file, &header, sizeof(elf32_header_t));
    if (status != sizeof(elf32_header_t))
        kernel_fatal_error("could not read elf file header");

    
}

