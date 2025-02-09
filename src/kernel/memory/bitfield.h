#ifndef SATAN_MEMORY_BITFIELD_H_
#define SATAN_MEMORY_BITFIELD_H_

#include <stdint.h>

int32_t bitfield_acquire_first(uint8_t *bitfields, uint32_t bitfield_count);
void bitfield_clear(uint8_t *bitfields, uint32_t bit_index);


#endif
