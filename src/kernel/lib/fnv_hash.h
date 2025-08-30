#ifndef SATAN_LIB_FNV_HASH_H_
#define SATAN_LIB_FNV_HASH_H_

#include <stdint.h>
#include <stddef.h>

uint32_t fnv1a_hash_32(const void *data, size_t len);
uint64_t fnv1a_hash_64(const void *data, size_t len);

#endif
