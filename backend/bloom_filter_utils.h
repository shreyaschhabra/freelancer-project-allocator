#ifndef BLOOM_FILTER_UTILS_H
#define BLOOM_FILTER_UTILS_H

#include "bloom_filter.h"
#include "utils.h"

// Initialize and populate a Bloom filter with all freelancer skills
typedef struct {
    BloomFilter filter;
    int initialized;
} GlobalBloom;

void populate_bloom_with_freelancer_skills(GlobalBloom* global_bloom, const Freelancer* freelancers, int num_freelancers);

#endif // BLOOM_FILTER_UTILS_H
