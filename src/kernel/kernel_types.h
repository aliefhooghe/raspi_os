#ifndef SATAN_KERNEL_TYPE_H_
#define SATAN_KERNEL_TYPE_H_

#include <stdint.h>

// user <-> kernel communications models
//
#define MAX_DIR_NAME_LEN (32u)

#define DT_REG 0u
#define DT_DIR 1u

typedef struct dirent {
  uint8_t d_type;  // 0 = file, 1 = directory
  char d_name[MAX_DIR_NAME_LEN];
} dirent;


#endif
