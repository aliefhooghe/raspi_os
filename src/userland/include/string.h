#ifndef SATAN_USERLAND_LIBC_STRING_H_
#define SATAN_USERLAND_LIBC_STRING_H_

#include <stddef.h>

void *memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size);

void *memset(
    void *destination,
    int c,
    size_t n);

void *memmove(
    void *destination,
    const void *source,
    size_t size);

size_t strlen(const char *);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dst, const char *src);

#endif
