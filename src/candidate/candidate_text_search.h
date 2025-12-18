#ifndef SEQUENTIAL_TEXT_SEARCH_H
#define SEQUENTIAL_TEXT_SEARCH_H

#include <string>
#include <vector>

#include "../timer.h"

std::vector<std::vector<int>>
find_candidate(const std::string &text,
               const std::vector<std::string> &queries);

#endif
