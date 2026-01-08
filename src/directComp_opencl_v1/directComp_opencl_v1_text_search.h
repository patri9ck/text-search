#ifndef DIRECT_COMPARE_OPENCL_V1_TEXT_SEARCH_H
#define DIRECT_COMPARE_OPENCL_V1_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer direct_compare_opencl_v1_timer;
#endif

std::vector<std::vector<size_t>>
direct_compare_opencl_v1(const std::string &text,
                         const std::vector<std::string> &queries);

#endif