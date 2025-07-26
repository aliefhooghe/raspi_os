#ifndef SATAN_STD_STR_H_
#define SATAN_STD_STR_H_

#include <stdint.h>
#include <stddef.h>

/**
 * tiny kernel pseudo libc
 */

void *_memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size);

void *_memset(
    void *destination,
    int c,
    size_t n);

void *_memmove(
    void *destination,
    const void *source,
    size_t size);

char *_strcpy(
    char *dst,
    const char *src);

int32_t _strcmp(
    const char *s1,
    const char *s2);

#endif
