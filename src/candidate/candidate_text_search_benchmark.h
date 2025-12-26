#ifndef SEQUENTIAL_TEXT_SEARCH_BENCHMARK_H
#define SEQUENTIAL_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer candidate_timer;

std::vector<std::vector<int>>
benchmark_candidate(const std::string &text,
                    const std::vector<std::string> &queries);

#endif
