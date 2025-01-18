#ifndef SATAN_STD_STR_H_
#define SATAN_STD_STR_H_

#include <stddef.h>

void *_memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size);

void *_memset(
    void *destination,
    int c,
    size_t n);

#endif
