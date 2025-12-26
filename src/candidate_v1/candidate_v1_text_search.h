#ifndef CANDIDATE_V1_TEXT_SEARCH_H
#define CANDIDATE_V1_TEXT_SEARCH_H

#include <string>
#include <vector>

#include "../timer.h"

std::vector<std::vector<int>>
find_candidate_v1(const std::string &text,
                  const std::vector<std::string> &queries);

#endif
