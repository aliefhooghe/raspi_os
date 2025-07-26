
#include <stdint.h>

#include "lib/str.h"

void *_memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size)
{
    uint8_t *dst = (uint8_t*)destination;
    const uint8_t *src = (const uint8_t*)source;

    for (uint32_t i = 0; i < size; ++i) {
        dst[i] = src[i];
    }

    return destination;
}


void *_memset(
    void *destination,
    int c,
    size_t size)
{
    uint8_t *dst = (uint8_t*)destination;
    for (uint32_t i = 0; i < size; ++i) {
        dst[i] = c;
    }
    return dst;
}

void *_memmove(
    void *destination,
    const void *source,
    size_t size)
{
    uint8_t *dst = (uint8_t*)destination;
    const uint8_t *src = (const uint8_t*)source;

    if (dst < src)
    {
        for (uint32_t i = 0; i < size; i++) {
            dst[i] = src[i];
        }
    }
    else {
        for (int32_t i = size - 1; i >= 0; i--) {
            dst[i] = src[i];
        }
    }

    return destination;
}

char *_strcpy(char *dst, const char *src)
{
    char *const ret = dst;
    while ((*dst++ = *src++));
    return ret;  
}

int32_t _strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && (*s1 == *s2)) {s1++; s2++;}
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}
