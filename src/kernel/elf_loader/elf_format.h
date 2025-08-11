#ifndef SATAN_ELF_FORMAT_H_
#define SATAN_ELF_FORMAT_H_

#include <stdint.h>

//
// sources:
// https://www.man7.org/linux/man-pages/man5/elf.5.html
// https://docs.oracle.com/cd/E19957-01/806-0641/chapter6-35342/index.html


// id_magic
#define ELF_MAGIC 0x464c457fu

// id_class
typedef enum {
    ELFCLASSNONE = 0,         // invalid
    ELFCLASS32 = 1,           // 32 bit object
    ELFCLASS64 = 2            // 64 bit object
} elf_class_t;

// data encoding
typedef enum {
   ELFDATANONE = 0,           // invalid
   ELFDATA2LSB = 1,           // litle endian
   ELFDATA2MSB = 2            // big endian
} elf_data_encoding_t;

// type
typedef enum {
    ET_NONE = 0,              // no file type
    ET_REL = 1,               // relocatable file
    ET_EXEC = 2,              // executable file
    ET_DYN = 3,               // shared object file
    ET_CORE = 4               // core file
} elf_type_t;

// machine
typedef enum {
    EM_NONE = 0,              // no machine
    EM_ARM = 40               // Advanced Risc Machine
} elf_machine_t;

typedef struct __attribute__((packed)) {
    // uint8_t	 ident[16];
    //
    //
    //

    // ident architecture independant
    uint32_t id_magic;         // 0x7f, e, l, f
    uint8_t id_class;          // 32/64 bits
    uint8_t id_data_encoding;  // big endian / little endian
    uint8_t id_elf_version;    // elf spec version
    uint8_t id_os_abi;         // operating system and abi
    uint8_t id_abi_version;    // abi version
    uint8_t __pad[7];          // padded with zero
 
    // 32 bit specific
    uint16_t type;         // exec/dyn lib
    uint16_t machine;      // proc architecture
    uint32_t version;      // file version (current or None)
    uint32_t entry;        // entry point virtual address (or zero if none) 
    uint32_t phoff;        // program header table file offset if any
    uint32_t shoff;        // section header table file offset if any
    uint32_t flags;        // proc files. unused
    uint16_t ehsize;       // elf header size
    uint16_t phentsize;    // program header table entry size
    uint16_t phnum;        // program header table entry count
    uint16_t shentsize;    // section header table entry size
    uint16_t shnum;        // section header table entry count
    uint16_t shstrndx;     // TODO: re read documentation :)
} elf32_header_t;

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddress;
    uint32_t paddress;
    uint32_t file_size;
    uint32_t mem_size;
    uint32_t flags;
    uint32_t align;
} elf32_program_header_t;

typedef struct __attribute__((packed)) {
   uint32_t name;
   uint32_t type;
   uint32_t flags;
   uint32_t addr;
   uint32_t offset;
   uint32_t size;
   uint32_t link;
   uint32_t info;
   uint32_t address_align;
   uint32_t entsize;
} elf32_section_header_t;


_Static_assert(
    sizeof(elf32_header_t) == 52, // todo: check
    "bad elf header size");

#endif
