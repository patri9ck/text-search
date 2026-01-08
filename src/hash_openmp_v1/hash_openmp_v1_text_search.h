#ifndef HASH_OPENMP_V1_TEXT_SEARCH_H
#define HASH_OPENMP_V1_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer hash_openmp_v1_timer;
#endif

std::vector<std::vector<size_t>>
find_hash_openmp_v1(const std::string &text, const std::vector<std::string> &queries);

#endif
