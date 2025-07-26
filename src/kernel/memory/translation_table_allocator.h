#ifndef SATAN_MEMORY_TRANSLATION_TABLE_ALLOCATOR_H_
#define SATAN_MEMORY_TRANSLATION_TABLE_ALLOCATOR_H_

#include <stdint.h>

void translation_table_allocator_init(void *memory_section);

uint32_t *translation_table_allocator_alloc(void);
void translation_table_allocator_free(uint32_t *);

#endif
