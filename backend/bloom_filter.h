#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <stdbool.h>
#include <stddef.h>

#define BLOOM_FILTER_SIZE 1024 // bits
#define BLOOM_HASH_COUNT 3

// Bloom filter structure
typedef struct {
    unsigned char bits[BLOOM_FILTER_SIZE / 8];
    size_t size;
    int hash_count;
} BloomFilter;

void bloom_init(BloomFilter* filter);
void bloom_add(BloomFilter* filter, const char* item);
bool bloom_check(const BloomFilter* filter, const char* item);
void bloom_free(BloomFilter* filter);

#endif // BLOOM_FILTER_H
