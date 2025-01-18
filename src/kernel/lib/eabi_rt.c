
#include "lib/str.h"

/**
 *  compiler runtime library
 */

void __aeabi_memcpy8(void *dest, const void *src, size_t n)
{
    _memcpy(dest, src, n);
}

void __aeabi_memcpy4(void *dest, const void *src, size_t n)
{
    _memcpy(dest, src, n);
}

void __aeabi_memcpy(void *dest, const void *src, size_t n)
{
    _memcpy(dest, src, n);
}

// void __aeabi_memmove8(void *dest, const void *src, size_t n)
// {
//     _memmove(dest, src, n);
// }

// void __aeabi_memmove4(void *dest, const void *src, size_t n)
// {
//     _memmove(dest, src, n);
// }

// void __aeabi_memmove(void *dest, const void *src, size_t n)
// {
//     _memmove(dest, src, n);
// }

void __aeabi_memset8(void *dest, size_t n, int c)
{
    _memset(dest, c, n);
}

void __aeabi_memset4(void *dest, size_t n, int c)
{
    _memset(dest, c, n);
}

void __aeabi_memset(void *dest, size_t n, int c)
{
    _memset(dest, c, n);
}

void __aeabi_memclr8(void *dest, size_t n)
{
    __aeabi_memset8(dest, n, 0);
}

void __aeabi_memclr4(void *dest, size_t n)
{
    __aeabi_memset4(dest, n, 0);
}

void __aeabi_memclr(void *dest, size_t n)
{
    __aeabi_memset(dest, n, 0);
}
