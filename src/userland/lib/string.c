#include <stdint.h>
#include <stddef.h>

#include "string.h"

// --

// eabi_rt.c
extern void __aeabi_memcpy(void *restrict dest, const void *restrict src, size_t n);
extern void __aeabi_memset(void *restrict dest, size_t n, int32_t c);
extern void __aeabi_memmove(void *dest, const void *src, size_t n);

void *memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size)
{
    __aeabi_memcpy(destination, source, size);
    return destination;
}

void *memset(
    void *destination,
    int c,
    size_t size)
{
    __aeabi_memset(destination, size, c);
    return destination;
}

void *memmove(
    void *destination,
    const void *source,
    size_t size)
{
    __aeabi_memmove(destination, source, size);
    return destination;
}

// --

size_t strlen(const char *s)
{
    size_t len = 0u;
    while (*(s++))
        len++;
    return len;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && (*s1 == *s2)) {s1++; s2++;}
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

char *strcpy(char *dst, const char *src)
{
    char *const ret = dst;
    while ((*dst++ = *src++));
    return ret;  
}

char *strcat(char *dst, const char *src)
{
    const size_t dst_len = strlen(dst);
    strcpy(dst + dst_len, src);
    return dst;
}
