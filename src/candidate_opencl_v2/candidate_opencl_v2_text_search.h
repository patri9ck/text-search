#ifndef CANDIDATE_OPENCL_V2_TEXT_SEARCH_H
#define CANDIDATE_OPENCL_V2_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer candidate_v3_timer;
#endif

std::vector<std::vector<size_t>>
find_candidate_opencl_v2(const std::string &text,
                         const std::vector<std::string> &queries);

#endif
