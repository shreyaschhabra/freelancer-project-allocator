#include "bloom_filter.h"
#include <string.h>
#include <stdlib.h>

// Simple djb2 hash
static unsigned int hash1(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

// Simple sdbm hash
static unsigned int hash2(const char* str) {
    unsigned int hash = 0;
    int c;
    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

// Simple lose lose hash
static unsigned int hash3(const char* str) {
    unsigned int hash = 0;
    int c;
    while ((c = *str++))
        hash += c;
    return hash;
}

void bloom_init(BloomFilter* filter) {
    memset(filter->bits, 0, sizeof(filter->bits));
    filter->size = BLOOM_FILTER_SIZE;
    filter->hash_count = BLOOM_HASH_COUNT;
}

void bloom_add(BloomFilter* filter, const char* item) {
    unsigned int hashes[BLOOM_HASH_COUNT];
    hashes[0] = hash1(item) % filter->size;
    hashes[1] = hash2(item) % filter->size;
    hashes[2] = hash3(item) % filter->size;
    for (int i = 0; i < filter->hash_count; i++) {
        filter->bits[hashes[i] / 8] |= (1 << (hashes[i] % 8));
    }
}

bool bloom_check(const BloomFilter* filter, const char* item) {
    unsigned int hashes[BLOOM_HASH_COUNT];
    hashes[0] = hash1(item) % filter->size;
    hashes[1] = hash2(item) % filter->size;
    hashes[2] = hash3(item) % filter->size;
    for (int i = 0; i < filter->hash_count; i++) {
        if (!(filter->bits[hashes[i] / 8] & (1 << (hashes[i] % 8)))) {
            return false;
        }
    }
    return true;
}

void bloom_free(BloomFilter* filter) {
    // No dynamic memory to free
}
