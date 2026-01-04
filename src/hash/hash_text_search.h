#ifndef HASH_TEXT_SEARCH_H
#define HASH_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
extern Timer hash_timer;
#endif

std::vector<std::vector<size_t>>
find_hash(const std::string &text, const std::vector<std::string> &queries);

#endif
