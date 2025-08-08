#ifndef SATAN_UTILS_H_
#define SATAN_UTILS_H_

#include <stddef.h>

void load_resource_as_file(
    const char *path,
    const void *resource_data,
    const size_t resource_size
);

#endif
