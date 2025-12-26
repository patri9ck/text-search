#ifndef HASH_TEXT_SEARCH_BENCHMARK_H
#define HASH_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer hash_timer;

std::vector<std::vector<int>>
benchmark_hash(const std::string &text,
               const std::vector<std::string> &queries);

#endif // HASH_TEXT_SEARCH_BENCHMARK_H
