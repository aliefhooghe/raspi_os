
#include "memory/allocator.h"
#include "usermode/libc/stdlib.h"

void *malloc(size_t size)
{
    return mem_alloc(size);
}

void free(void  *ptr)
{
    mem_free(ptr);
}
