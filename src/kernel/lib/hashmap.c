#include <memory/memory_allocator.h>

#include "hashmap.h"
#include "fnv_hash.h"
#include "kernel.h"
#include "lib/str.h"

#define HASH_MAP_DEFAULT_CAPACITY (64u)
#define HASH_MAP_MAX_KEY_SIZE (16u)

typedef struct {
    uint8_t key[HASH_MAP_MAX_KEY_SIZE];
    uint32_t key_size;
    uint32_t key_hash;
    void *value;
} kv_pair_t;

struct hash_map {
    kv_pair_t* pairs;
    size_t capacity;
};


int hash_map_create(
    hash_map_t **h)
{
    hash_map_t *hm = memory_calloc(sizeof(hash_map_t));
    if (hm == NULL)
        return -1;

    hm->capacity = HASH_MAP_DEFAULT_CAPACITY;
    hm->pairs = memory_calloc(hm->capacity * sizeof(kv_pair_t));
    if (hm->pairs == NULL)
    {
        memory_free(hm);
        return -1;
    }

    *h = hm;
    return -1;
}

int hash_map_free(
    hash_map_t *h)
{
    if (h == NULL)
        return -1;
    if (h->pairs != NULL)
        memory_free(h->pairs);
    memory_free(h);
    return 0;
}

void *hash_map_get(
    hash_map_t *h,
    void *key, size_t keysize)
{
    const size_t hash = fnv1a_hash_32(key, keysize);
    size_t index = hash % h->capacity;

    for (; index < h->capacity; index++) {
        const kv_pair_t *pair = &h->pairs[index];
        if (pair->value == NULL) {
            return NULL;
        }
        else if (pair->key_hash == hash &&
                 pair->key_size == keysize &&
                 _memcmp(pair->key, key, keysize) == 0) {
            // only compare keys if hash and size matches
            return h->pairs[index].value;
        }
    }

    return NULL;
}

int hash_map_set(
    hash_map_t *h,
    void *key, size_t keysize,
    void *value)
{
    if (value == NULL)
        kernel_fatal_error("cannot insert NULL in hashmap");
    else if (keysize > HASH_MAP_MAX_KEY_SIZE)
        kernel_fatal_error("hashmap: keysize exeed max supported limit");

    const size_t hash = fnv1a_hash_32(key, keysize);
    size_t index = hash % h->capacity;

    for (; index < h->capacity; index++) {
        const kv_pair_t *pair = &h->pairs[index];
        if (pair->value == NULL ||
            (pair->key_hash == hash &&
             pair->key_size == keysize &&
             _memcmp(pair->key, key, keysize) == 0))
        {
            break;
        }
    }

    if (index == h->capacity)
        return -1;

    kv_pair_t *pair = &h->pairs[index];
    _memcpy(pair->key, key, keysize);
    pair->key_size = keysize;
    pair->key_hash = hash;
    pair->value = value;
    return 0;
}
