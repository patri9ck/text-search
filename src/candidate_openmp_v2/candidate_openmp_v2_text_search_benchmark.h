#ifndef CANDIDATE_OPENMP_V2_TEXT_SEARCH_BENCHMARK_H
#define CANDIDATE_OPENMP_V2_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer candidate_openmp_v2_timer;

std::vector<std::vector<int>>
benchmark_candidate_openmp_v2(const std::string &text,
                              const std::vector<std::string> &queries);

#endif
