
#include <stdint.h>
#include <stddef.h>

// 
//  Compiler Runtime Library
//  used by both kernel and userland

//
// Memory management
//

void __aeabi_memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    uint8_t *bd = (uint8_t*)dest;
    const uint8_t *bs = (const uint8_t*)src;
    for (size_t i = 0; i < n; ++i) {
        bd[i] = bs[i];
    }
}

void __aeabi_memcpy4(void *restrict dest, const void *restrict src, size_t n)
{
    __aeabi_memcpy(dest, src, n);
}

void __aeabi_memcpy8(void *restrict dest, const void *restrict src, size_t n)
{
    __aeabi_memcpy(dest, src, n);
}

// --

void __aeabi_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *bd = (uint8_t*)dest;
    const uint8_t *bs = (const uint8_t*)src;

    if (bd < bs)
    {
        for (uint32_t i = 0u; i < n; i++) {
            bd[i] = bs[i];
        }
    }
    else {
        for (int32_t i = n - 1; i >= 0; i--) {
            bd[i] = bs[i];
        }
    }

}

void __aeabi_memmove4(void *dest, const void *src, size_t n)
{
    __aeabi_memmove(dest, src, n);
}

void __aeabi_memmove8(void *dest, const void *src, size_t n)
{
    __aeabi_memmove(dest, src, n);
}

// --

void __aeabi_memset(void *dst, size_t n, int32_t c)
{
    uint8_t *bs = (uint8_t*)dst;
    for (uint32_t i = 0; i < n; ++i) {
        bs[i] = c;
    }
}

void __aeabi_memset4(void *dest, size_t n, int32_t c)
{
    __aeabi_memset(dest, c, n);
}

void __aeabi_memset8(void *dest, size_t n, int32_t c)
{
    __aeabi_memset(dest, c, n);
}

// --

void __aeabi_memclr(void *dest, size_t n)
{
    __aeabi_memset(dest, n, 0);
}

void __aeabi_memclr4(void *dest, size_t n)
{
    __aeabi_memset4(dest, n, 0);
}

void __aeabi_memclr8(void *dest, size_t n)
{
    __aeabi_memset8(dest, n, 0);
}

//
// Math
//

// signed division
// int32_t __aeabi_idiv(int32_t numerator, int32_t denominator)
// {
//     if (denominator == 0)
//         return 0; // TODO: trap the divide by zero

    
// }

// int32_t __aeabi_idivmod(int32_t numerator, int32_t denominator)
// {
    
// }

// unsigned division
uint32_t __aeabi_uidiv(uint32_t numerator, uint32_t denominator)
{
    if (denominator == 0)
        return 0; // TODO: trap the divide by zero

    uint32_t quo = 0;
    while (numerator >= denominator) {
        numerator -= denominator;
        quo++;
    }

    return quo;
}

// uint32_t __aeabi_uidivmod(uint32_t numerator, uint32_t denominator)
// {
    
// }
