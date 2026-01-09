#ifndef TEXT_SEARCH_NAIVE_V1_TEXT_SEARCH_H
#define TEXT_SEARCH_NAIVE_V1_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
#include "../timer.h"
extern Timer naive_v1_timer;
#endif

std::vector<std::vector<size_t>>
naive_v1(const std::string &text, const std::vector<std::string> &queries);

#endif //TEXT_SEARCH_NAIVE_V1_TEXT_SEARCH_H
