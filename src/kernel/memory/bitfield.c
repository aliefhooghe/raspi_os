
#include "bitfield.h"
#include "hardware/mini_uart.h"
#include "kernel.h"

int32_t bitfield_acquire_first(uint8_t *bitfields, uint32_t bitfield_count)
{
    for (uint32_t bitfield_index = 0u;
        bitfield_index < bitfield_count;
        bitfield_index++)
    {
        uint8_t *bitfield = &bitfields[bitfield_index];
        if (*bitfield == 0xFFu)
            continue;
        for (uint32_t bit_index = 0u; bit_index < 8; bit_index++)
        {
            if (0 == ((1 << bit_index) & *bitfield))
            {
                // set the alloc bit
                *bitfield |= (1u << bit_index);

                // return the global bit index
                return bit_index + 8 * bitfield_index;
            }
        }
    }

    mini_uart_kernel_log("bitfield: failed allocation (bitfield_cound=%u)", bitfield_count);
    return -1;
}

void bitfield_clear(
    uint8_t *bitfields,
    uint32_t bitfield_count,
    uint32_t bit_index)
{
    const uint32_t bitfield_index = bit_index >> 3u;
    const uint32_t local_bit_index = bit_index & 0x7u;

    if (bitfield_index > bitfield_count)
        return;

    bitfields[bitfield_index] &= ~(1u << local_bit_index);
}

int32_t bitfield_bit(
    const uint8_t *bitfields,
    uint32_t bitfield_count,
    uint32_t bit_index)
{
    const uint32_t bitfield_index = bit_index >> 3u;
    const uint32_t local_bit_index = bit_index & 0x7u;

    if (bitfield_index > bitfield_count)
        return 0;

    return !!(bitfields[bitfield_index] & (1u << local_bit_index));
}

void *bitfield_allocator_alloc(
    void *base, size_t size,
    uint8_t *bitfields,
    uint32_t bitfield_count)
{
    const int32_t index = bitfield_acquire_first(
        bitfields,
        bitfield_count);
    if (index < 0)
        return NULL;
    else
        return (void*)((uint8_t*)base + index * size);
}


void bitfield_allocator_free(
    void *base, size_t size,
    uint8_t *bitfields, uint32_t bitfield_count,
    void *ptr)
{
    const uint8_t *const u8base = (uint8_t*)base;
    const uint8_t *const u8ptr = (uint8_t*)ptr;

    if (u8ptr < u8base)
        kernel_fatal_error(
            "bitfield allocator: free: tried to free memory before base");

    const size_t u8offset = u8ptr - u8base;

    // TODO: division is not optimized enough, we could avoid it ?
    const size_t index = u8offset / size;

    bitfield_clear(bitfields, bitfield_count, index);
}
