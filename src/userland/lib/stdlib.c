
#include <stdint.h>
#include "stdlib.h"

// process section is 0x00800000u -> 0x00900000u = 1Mb
// pseudo heap: 0x00880000u -> 0x00900000u
// @see scheduler.c

#define HEAP_CURSOR 0x00880100u
#define HEAP_START  0x00881000u
#define HEAP_END    0x00900000u


void *malloc(size_t size)
{
    size_t *cursor = (size_t*)HEAP_CURSOR;
    if (*cursor == 0u)
        *cursor = HEAP_START;   
    const size_t old_cursor = *cursor;
    const size_t new_cursor = (old_cursor + size + sizeof(uint32_t) - 1u) | 0x3u;
    if (new_cursor > HEAP_END)
        return NULL;
    *cursor = new_cursor;
    return (void*)old_cursor;
}

void free(void  *ptr)
{
    // TODO: implement a real error
    (void)ptr;
}

