#ifndef SATAN_USERLAND_LIBC_STRING_H_
#define SATAN_USERLAND_LIBC_STRING_H_

#include <stddef.h>

size_t strlen(const char *);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dst, const char *src);
void *memcpy(
    void * restrict destination, const void * restrict source,
    size_t size);

#endif
