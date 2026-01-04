#ifndef CANDIDATE_V2_TEXT_SEARCH_H
#define CANDIDATE_V2_TEXT_SEARCH_H

#include <string>
#include <vector>

#include "../timer.h"

#ifdef BENCHMARK
extern Timer candidate_v2_timer;
#endif

std::vector<std::vector<size_t>>
find_candidate_v2(const std::string &text,
                  const std::vector<std::string> &queries);

#endif
