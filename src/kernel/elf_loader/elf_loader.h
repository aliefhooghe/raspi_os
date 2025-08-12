#ifndef SATAN_ELF_LOADER_H_
#define SATAN_ELF_LOADER_H_

#include <stdint.h>

#include "vfs/inode.h"
#include "elf_format.h"

typedef struct {
    file_t *file;
    elf32_header_t header;
} elf32_file_t;

typedef struct {
    elf32_file_t *elf;
    size_t current_section;
} elf32_program_header_iterator_t;


int32_t elf32_open(const char *path, elf32_file_t *file);
int32_t elf32_close(elf32_file_t *file);

elf32_program_header_iterator_t elf32_init_program_header_iterator(
    elf32_file_t* file);
ssize_t elf32_program_header_iterator_read_next(
    elf32_program_header_iterator_t *iterator,
    elf32_program_header_t *header);


#endif
