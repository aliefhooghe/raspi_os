#ifndef SATAN_LIB_HASHMAP_H_
#define SATAN_LIB_HASHMAP_H_

#include <stddef.h>

typedef struct hash_map hash_map_t;

int hash_map_create(
    hash_map_t **h
);

int hash_map_free(
    hash_map_t *h
);

void *hash_map_get(
    hash_map_t *h,
    void *key, size_t keysize
);

int hash_map_set(
    hash_map_t *h,
    void *key, size_t keysize,
    void *value
);

#endif
