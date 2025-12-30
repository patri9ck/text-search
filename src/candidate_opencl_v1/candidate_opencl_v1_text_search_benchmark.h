#ifndef TEXT_SEARCH_CANDIDATE_OPENCL_V1_TEXT_SEARCH_BENCHMARK_H
#define TEXT_SEARCH_CANDIDATE_OPENCL_V1_TEXT_SEARCH_BENCHMARK_H

#include "../timer.h"

extern Timer candidate_opencl_v1_timer;

std::vector<std::vector<int>>
benchmark_candidate_opencl_v1(const std::string &text,
                       const std::vector<std::string> &queries);


#endif //TEXT_SEARCH_CANDIDATE_OPENCL_V1_TEXT_SEARCH_BENCHMARK_H
