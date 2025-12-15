#ifndef STD_TEXT_SEARCH_BENCHMARK_H
#define STD_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer std_timer;

void benchmark_std(const std::string &text,
                   const std::vector<std::string> &queries);

#endif // STD_TEXT_SEARCH_BENCHMARK_H
