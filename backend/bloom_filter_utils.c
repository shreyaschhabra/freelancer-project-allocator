#include "bloom_filter_utils.h"
#include <string.h>

void populate_bloom_with_freelancer_skills(GlobalBloom* global_bloom, const Freelancer* freelancers, int num_freelancers) {
    if (global_bloom->initialized) return;
    bloom_init(&global_bloom->filter);
    for (int i = 0; i < num_freelancers; i++) {
        for (int j = 0; j < freelancers[i].num_skills; j++) {
            bloom_add(&global_bloom->filter, freelancers[i].skills[j]);
        }
    }
    global_bloom->initialized = 1;
}
