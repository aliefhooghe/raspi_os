#ifndef SATAN_USERMODE_LIBC_DIRENT_H_
#define SATAN_USERMODE_LIBC_DIRENT_H_

// we share dirent with the kernel
#include "kernel_types.h"

typedef struct DIR DIR;

DIR *opendir(const char *name);
int dirfd(DIR *dirp);

struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);
long telldir(DIR *dirp);
void seekdir(DIR *dirp, long loc);

#endif
