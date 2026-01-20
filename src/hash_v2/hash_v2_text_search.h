#ifndef TEXT_SEARCH_HASH_V2_TEXT_SEARCH_H
#define TEXT_SEARCH_HASH_V2_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
#include "../timer.h"
extern Timer hash_v2_timer;
#endif

std::vector<std::vector<size_t>>
hash_v2(const std::string &text, const std::vector<std::string> &queries);

#endif //TEXT_SEARCH_HASH_V2_TEXT_SEARCH_H
