#ifndef SATAN_USERMODE_LIBC_STDLIB_H_
#define SATAN_USERMODE_LIBC_STDLIB_H_

#include <stddef.h>

void *malloc(size_t size);
void free(void  *ptr);

#endif
