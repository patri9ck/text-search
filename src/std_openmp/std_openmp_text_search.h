#ifndef STD_OPENMP_TEXT_SEARCH_H
#define STD_OPENMP_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer std_openmp_timer;
#endif

std::vector<std::vector<size_t>>
find_std_openmp(const std::string &text,
                const std::vector<std::string> &queries);

#endif
