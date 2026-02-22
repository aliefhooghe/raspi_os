
#include "log/log.h"
#include "kernel_types.h"
#include "lib/str.h"
#include "vfs/vfs.h"

#include "elf_format.h"
#include "elf_loader.h"


int32_t _elf32_check_compat(const elf32_header_t *header)
{
    return (
        header->id_magic == ELF_MAGIC &&   // this is an elf file
        header->id_class == ELFCLASS32 &&  // 32bit addresses
        header->id_data_encoding == ELFDATA2LSB && // little endian
        header->machine == EM_ARM && // arm cpu architecture

        // future elf could allocate larger size
        header->phentsize >= sizeof(elf32_program_header_t) &&
        header->shentsize >= sizeof(elf32_section_header_t)
    );
}

int32_t elf32_open(const char *path, elf32_file_t *file)
{
    // open elf file
    file->file = vfs_file_open(path, 0u, 0u);
    if (file->file == NULL) {
        kernel_log(
            "elf32_open: failed to open path %s", path);
        return -1;
    }

    // read elf header
    _memset(&file->header, 0u, sizeof(elf32_header_t));
    const ssize_t status = vfs_file_read(
        file->file, &file->header, sizeof(elf32_header_t));

    // validate compatibility
    if (status != sizeof(elf32_header_t) ||
        !_elf32_check_compat(&file->header))
    {
        kernel_log(
            "elf32_open: invalid elf file at %s",
            path);
        elf32_close(file);
        return -1;
    }

    return 0;
}

int32_t elf32_close(elf32_file_t *file)
{
    const int32_t status = vfs_file_close(file->file);
    if (status < 0)
        return status;
    file->file = NULL;
    return 0;
}

elf32_program_header_iterator_t elf32_init_program_header_iterator(
    elf32_file_t* file)
{
    const elf32_program_header_iterator_t it = {
        .elf = file,
        .current_section = 0u
    };
    return it;
}

ssize_t elf32_program_header_iterator_read_next(
    elf32_program_header_iterator_t *iterator,
    elf32_program_header_t *header)
{
    if (iterator->current_section >= iterator->elf->header.phnum)
        return 0;

    // seek to the program header offset
    const size_t current_section = iterator->current_section++;
    const off_t offset =
        iterator->elf->header.phoff +
        current_section * iterator->elf->header.phentsize;
    const off_t actual_offset =
        vfs_file_lseek(iterator->elf->file, offset, SEEK_SET);
    if (actual_offset != offset)
        return -1;

    // read program header
    const ssize_t status =
        vfs_file_read(iterator->elf->file,
                      header,
                      sizeof(elf32_program_header_t));
    if (status < 0)
        return status;
    else if (status != sizeof(elf32_program_header_t))
        return -1;
    else
        return 1;
}
