
#include "hardware/mini_uart.h"
#include "kernel.h"
#include "lib/str.h"
#include "vfs/vfs.h"
#include "elf_format.h"


void elf_test(const char *path)
{
    mini_uart_kernel_log("elf test: read file %s", path);
    file_t *file = vfs_file_open(path, 0u, 0u);
    if (file == NULL)
        kernel_fatal_error("could not open elf file");

    mini_uart_kernel_log("elf test: read header");
    elf32_header_t header;
    _memset(&header, 0u, sizeof(header));

    const ssize_t status = vfs_file_read(file, &header, sizeof(elf32_header_t));
    if (status != sizeof(elf32_header_t))
        kernel_fatal_error("could not read elf file header");

    // Check if this is an elf file
    mini_uart_kernel_log("elf test: check magic: 0x%x", header.id_magic);
    if (header.id_magic != ELF_MAGIC)
        kernel_fatal_error("bad elf header magic value");

    // Check if elf class is 32 bit
    mini_uart_kernel_log("elf test: check class: 0x%x", header.id_class);
    if (header.id_class != ELFCLASS32)
        kernel_fatal_error("bad elf class");
    
    // Check if data encoding is little endian
    mini_uart_kernel_log("elf test: check encoding: 0x%x", header.id_data_encoding);
    if (header.id_data_encoding != ELFDATA2LSB)
        kernel_fatal_error("bad data encoding");

    // Check machine
    mini_uart_kernel_log("elf test: check machine: 0x%x", header.machine);
    if (header.machine!= EM_ARM)
        kernel_fatal_error("bad machine");

    // 
    mini_uart_kernel_log("elf test: elf type: 0x%x", header.type);
    
    mini_uart_kernel_log("elf test: Done.");
    vfs_file_close(file);
}

