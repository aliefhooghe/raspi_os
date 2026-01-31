
#include <stdint.h>
#include "lib/str.h"

// eabi_rt.c
extern void __aeabi_memcpy(void *restrict dest, const void *restrict src, size_t n);
extern void __aeabi_memset(void *restrict dest, size_t n, int32_t c);
extern void __aeabi_memmove(void *dest, const void *src, size_t n);

void *_memcpy(
    void *restrict destination,
    const void *restrict source,
    size_t size)
{
    __aeabi_memcpy(destination, source, size);
    return destination;
}

void *_memset(
    void *destination,
    int c,
    size_t size)
{
    __aeabi_memset(destination, size, c);
    return destination;
}

void *_memmove(
    void *destination,
    const void *source,
    size_t size)
{
    __aeabi_memmove(destination, source, size);
    return destination;
}


int32_t _memcmp(
    const void *s1,
    const void *s2,
    size_t n)
{
    const uint8_t *sb1 = (uint8_t*)s1;
    const uint8_t *sb2 = (uint8_t*)s2;

    size_t index = 0u;
    while (++index < n && (*sb1 == *sb2)) {
        sb1++; sb2++;
    }

    return (*sb1 - *sb2);
}

size_t _strlen(const char *s)
{
    size_t size = 0u;
    while (*s++) size++;
    return size;
}

char *_strcpy(char *dst, const char *src)
{
    char *const ret = dst;
    while ((*dst++ = *src++));
    return ret;  
}

char *_strcat(char *dst, const char *src)
{
    const size_t dst_len = _strlen(dst);
    _strcpy(dst + dst_len, src);
    return dst;
}

int32_t _strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && (*s1 == *s2)) {s1++; s2++;}
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

int32_t _strncmp(const char* s1, const char* s2, size_t count)
{
    size_t index = 0u;
    while (++index< count && *s1 != '\0' && (*s1 == *s2)) {s1++; s2++;}
    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

const char* _strchr(const char* str, char ch)
{
    str--;
    while (*++str) {
        if (*str == ch)
            return str;
    }
    return NULL;
}

const char *_strrchr(const char *str, char ch)
{
    const char *last = NULL;
    str--;
    while (*++str) {
        if (*str == ch)
            last = str;
    }
    return last;
}
