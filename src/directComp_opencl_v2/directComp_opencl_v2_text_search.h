#ifndef DIRECT_COMPARE_OPENCL_V2_TEXT_SEARCH_H
#define DIRECT_COMPARE_OPENCL_V2_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer direct_compare_opencl_v2_timer;
#endif

std::vector<std::vector<size_t>>
direct_compare_opencl_v2(const std::string &text,
                         const std::vector<std::string> &queries);

#endif