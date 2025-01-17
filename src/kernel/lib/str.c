
#include <stdint.h>
#include <std/str.h>

void *memcpy(
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
