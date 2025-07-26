#ifndef SATAN_MEMORY_SECTION_ALLOCATOR_H_
#define SATAN_MEMORY_SECTION_ALLOCATOR_H_

#include <stdint.h>

// One section size is 1Mb MMU_SECTION_SIZE

void section_allocator_init(uint32_t sections_base);
void *section_allocator_alloc(void);
void section_allocator_free(void *section);

#endif
