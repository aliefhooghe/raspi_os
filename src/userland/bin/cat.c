
#include <stdio.h>

int main(int argc, const char *argv[])
{
    if (argc != 1)
    {
        printf("usage: cat <path>\n");
        return 1;
    }

    const char *path = argv[0];
    FILE *fd = fopen(path, "r");
    if (fd == NULL)
    {
        printf("no file at '%s'\n", path);
        return 1;
    }

    char buffer[512];
    int sz = 0;
    while ((sz = fread(buffer, 512, 1, fd))) {
        fwrite(buffer, sz, 1, stdout);
    }
    fflush(stdout);

    fclose(fd);
    return 0;
}
