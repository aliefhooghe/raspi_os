
#include "fnv_hash.h"

//
// reference: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// 

#define FNV_HASH_PRIME_32        0x01000193u
#define FNV_HASH_OFFSET_BASIS_32 0x811C9DC5u

#define FNV_HASH_PRIME_64        0x00000100000001B3ull
#define FNV_HASH_OFFSET_BASIS_64 0xCBF29CE484222325ull

uint32_t fnv1a_hash_32(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;

    uint32_t h = FNV_HASH_OFFSET_BASIS_32;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= FNV_HASH_PRIME_32;
    }

    return h;
}

uint64_t fnv1a_hash_64(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;

    uint64_t h = FNV_HASH_OFFSET_BASIS_64;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= FNV_HASH_PRIME_64;
    }

    return h;
}
