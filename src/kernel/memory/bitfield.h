#ifndef SATAN_MEMORY_BITFIELD_H_
#define SATAN_MEMORY_BITFIELD_H_

#include <stddef.h>
#include <stdint.h>

//
//  Raw bitfields
// 
int32_t bitfield_acquire_first(
    uint8_t *bitfields,
    uint32_t bitfield_count);

void bitfield_clear(
    uint8_t *bitfields,
    uint32_t bitfield_count,
    uint32_t bit_index);

int32_t bitfield_bit(
    const uint8_t *bitfields,
    uint32_t bitfield_count,
    uint32_t bit_index);

//
//  Allocator based on bitfields
// 
void *bitfield_allocator_alloc(
    void *base, size_t size,
    uint8_t *bitfields,
    uint32_t bitfield_count);

void bitfield_allocator_free(
    void *base, size_t size,
    uint8_t *bitfields, uint32_t bitfield_count,
    void *ptr);


//
//  Allocator template
//
#define BITFIELD_COUNT(type, count) (count / 8)
#define BITFIELD_ALLOCATABLE_SIZE(type, count) (sizeof(type) * count)

// Note: allocator structure is supposed to be initialized
#define DEF_BITFIELD_ALLOCATOR(type, count) \
    _Static_assert( \
        count%8 == 0, \
        "bitfield allocator expect a multiple of 8 for count: type=" #type); \
    typedef struct { \
        type *base;  \
        uint8_t bitfields[BITFIELD_COUNT(type, count)]; \
    } type##_x_##count##_bitfield_allocator_t; \
    \
    type *type##_x_##count##_bitfield_alloc( \
        type##_x_##count##_bitfield_allocator_t *allocator) \
    { \
        return bitfield_allocator_alloc( \
            allocator->base, sizeof(type), \
            allocator->bitfields, BITFIELD_COUNT(type, count)); \
    } \
    \
    void type##_x_##count##_bitfield_free( \
        type##_x_##count##_bitfield_allocator_t *allocator, \
        type *ptr) \
    { \
        bitfield_allocator_free( \
            allocator->base, sizeof(type), \
            allocator->bitfields, BITFIELD_COUNT(type, count), \
            ptr); \
    }


#endif
