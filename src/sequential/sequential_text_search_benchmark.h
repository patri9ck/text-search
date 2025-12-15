#ifndef SEQUENTIAL_TEXT_SEARCH_BENCHMARK_H
#define SEQUENTIAL_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer sequential_timer;

void benchmark_sequential(const std::string &text,
                          const std::vector<std::string> &queries);

#endif
