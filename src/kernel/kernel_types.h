#ifndef SATAN_KERNEL_TYPE_H_
#define SATAN_KERNEL_TYPE_H_

#include <stddef.h>
#include <stdint.h>

// user <-> kernel communications models
//
typedef int32_t ssize_t;
typedef int32_t off_t;
typedef int64_t loff_t;

typedef uint32_t dev_t;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

//
// inode mode
typedef uint32_t mode_t;

#define S_IFREG	0x8000u
#define S_IFDIR	0x4000u
#define S_IFCHR	0x2000u
#define S_IFBLK	0x6000u
#define S_IFIFO	0x1000u
#define S_IFSOCK	0xC000u
#define S_IFLNK	0xA000u
#define S_IFMT 0xF000

// 
// Directory entities (readdir)
#define DT_UNKNOWN  0u
#define DT_FIFO     1u
#define DT_CHR      2u
#define DT_DIR      4u
#define DT_BLK      6u
#define DT_REG      8u
#define DT_LNK      10u
#define DT_SOCK     12u

#define MAX_DIR_NAME_LEN (31u)
typedef struct dirent {
  uint8_t d_type;  // 0 = file, 1 = directory
  char d_name[MAX_DIR_NAME_LEN];
} dirent;


#endif
