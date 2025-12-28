#ifndef CANDIDATE_OPENCL_V2_TEXT_SEARCH_BENCHMARK_H
#define CANDIDATE_OPENCL_V2_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer candidate_v3_timer;

std::vector<std::vector<int>>
benchmark_candidate_v3(const std::string &text,
                       const std::vector<std::string> &queries);

#endif
