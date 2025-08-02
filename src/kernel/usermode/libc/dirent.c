
#include "dirent.h"
#include "kernel_types.h"
#include "usermode/usr_syscalls.h"
#include "usermode/libc/stdlib.h"

struct DIR {
    int fd;
    dirent lastread;
};

_Static_assert(sizeof(dirent)==32, "sizeof(dirent)");

DIR *opendir(const char *name)
{
    int fd = usr_syscall_open(name, 0u, 0u); // todo: mode O_DIRECTORY
    if (fd == -1)
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
    const size_t size = usr_syscall_read(dirp->fd, &dirp->lastread, sizeof(dirent));
    if (size != sizeof(dirent))
        return NULL;
    return &dirp->lastread;
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
    const off_t offset = usr_syscall_lseek(dirp->fd, 0, SEEK_CUR);
    return offset >> 0x5;
}

void seekdir(DIR *dirp, long loc)
{
    const off_t offset = loc << 0x5;
    usr_syscall_lseek(dirp->fd, offset, SEEK_SET);
}

