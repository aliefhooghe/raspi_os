#include "dirent.h"
#include "stdio.h"

static int ls(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("ls: directory %s does not exists\n", path);
        return 1;
    }

    struct dirent *entity = NULL;
    while ((entity = readdir(dir))) {
        printf(" %s (%s)\n",
            entity->d_name,
                entity->d_type == DT_DIR ? "dir" :
                entity->d_type == DT_REG ? "reg" :
                entity->d_type == DT_CHR ? "character device" :
                entity->d_type == DT_BLK ? "block device" :
                "unknown");
    }

    closedir(dir);
    return 0;
}

int main(int argc, const char *argv[])
{
    if (argc > 1)
    {
        printf("usage: ls [path]\n");
        return 1;
    }

    const char *path = argc == 0 ? "/" : argv[0];
    return ls(path);
}
