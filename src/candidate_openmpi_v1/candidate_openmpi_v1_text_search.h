#ifndef CANDIDATE_OPENMPI_V3_TEXT_SEARCH_H
#define CANDIDATE_OPENMPI_V3_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer candidate_openmpi_v1_timer;
#endif

std::vector<std::vector<size_t>>
find_candidate_openmpi_v1(const std::string &text,
                  const std::vector<std::string> &queries);

#endif
