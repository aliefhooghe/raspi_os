#ifndef SATAN_UTILS_H_
#define SATAN_UTILS_H_

#include "kernel_types.h"
#include <stddef.h>

void load_resource_as_file(
    const char *path,
    const void *resource_data,
    const size_t resource_size
);

off_t off_t_min(off_t a, off_t b);
off_t off_t_max(off_t a, off_t b);
size_t size_t_max(size_t a, size_t b);
size_t size_t_min(size_t a, size_t b);

#endif
