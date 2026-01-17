
#include "dirent.h"
#include "kernel_types.h"
#include "usr_syscalls.h"
#include "stdlib.h"

struct DIR {
    int fd;
    dirent last_read;
};

DIR *opendir(const char *name)
{
    const int fd = usr_syscall_open(name, O_DIRECTORY, 0u);
    if (fd < 0)
        return NULL;

    DIR *dir = malloc(sizeof(DIR));
    if (dir == NULL) {
        usr_syscall_close(fd);
        return NULL;
    }

    dir->fd = fd;
    return dir;    
}

int dirfd(DIR *dirp)
{
    return dirp->fd;
}

struct dirent *readdir(DIR *dirp)
{
    const size_t count = usr_syscall_readdir(dirp->fd, &dirp->last_read, 1u);
    if (count != 1u)
        return NULL;
    return &dirp->last_read;
}

int closedir(DIR *dirp)
{
    int fd = dirfd(dirp);
    free(dirp);
    return usr_syscall_close(fd);
}

void rewinddir(DIR *dirp)
{
    usr_syscall_lseek(dirp->fd, 0, SEEK_SET);
}

long telldir(DIR *dirp)
{
    return usr_syscall_lseek(dirp->fd, 0, SEEK_CUR);
}

void seekdir(DIR *dirp, long loc)
{
    usr_syscall_lseek(dirp->fd, loc, SEEK_SET);
}

