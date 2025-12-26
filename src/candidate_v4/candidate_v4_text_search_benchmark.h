#ifndef CANDIDATE_V4_TEXT_SEARCH_BENCHMARK_H
#define CANDIDATE_V4_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer candidate_v4_timer;

std::vector<std::vector<int>>
benchmark_candidate_v4(const std::string &text,
                       const std::vector<std::string> &queries);

#endif
