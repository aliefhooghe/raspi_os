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

#define DEV_MAJOR(dev) ((uint16_t)((dev & 0xFF00u) >> 16))
#define DEV_MINOR(dev) ((uint16_t)(dev & 0xFFu))

//
// open flags
// #define O_RDONLY     0x000000   // Read-only
// #define O_WRONLY     0x000001   // Write-only
// #define O_RDWR       0x000002   // Read/write

#define O_CREAT      0x000040   // Create if not exists
#define O_EXCL       0x000080   // Fail if exists (w/ O_CREAT)
// #define O_NOCTTY     0x000100   // No TTY control
// #define O_TRUNC      0x000200   // Truncate if exists
// #define O_APPEND     0x000400   // Write at end
// #define O_NONBLOCK   0x000800   // Non-blocking
// #define O_DSYNC      0x001000   // Sync data only
// #define O_SYNC       0x101000   // Sync data + metadata
// #define O_RSYNC      0x101000   // Sync read like O_SYNC
#define O_DIRECTORY  0x020000   // Must be a directory
// #define O_NOFOLLOW   0x040000   // Don't follow symlinks
// #define O_CLOEXEC    0x080000   // Close on exec

//
// seek whence
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

//
// inode mode
typedef uint32_t mode_t;

#define S_IFREG     0x8000u  // regular file
#define S_IFDIR     0x4000u  // directory
#define S_IFCHR     0x2000u  // character device file
#define S_IFBLK     0x6000u  // block device file
#define S_IFIFO     0x1000u
#define S_IFSOCK    0xC000u
#define S_IFLNK     0xA000u

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
