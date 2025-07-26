
#include "bitfield.h"

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

    return -1;
}

void bitfield_clear(uint8_t *bitfields, uint32_t bit_index)
{
    const uint32_t bitfield_index = bit_index >> 3u;
    const uint32_t local_bit_index = bit_index & 0x7u;
    bitfields[bitfield_index] &= ~(1u << local_bit_index);
}
