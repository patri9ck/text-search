#ifndef CANDIDATE_OPENMP_V2_TEXT_SEARCH_H
#define CANDIDATE_OPENMP_V2_TEXT_SEARCH_H

#include <string>
#include <vector>

#include "../timer.h"

std::vector<std::vector<int>>
find_candidate_openmp_v2(const std::string &text,
                  const std::vector<std::string> &queries);

#endif
