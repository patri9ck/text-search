#ifndef DIRECT_OPENCL_TEXT_SEARCH_H
#define DIRECT_OPENCL_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer direct_opencl_timer;
#endif

std::vector<std::vector<size_t>>
find_direct_opencl(const std::string &text,
                   const std::vector<std::string> &queries);

#endif