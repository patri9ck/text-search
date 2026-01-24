#ifndef CANDIDATE_MPI_TEXT_SEARCH_H
#define CANDIDATE_MPI_TEXT_SEARCH_H

#include <string>
#include <vector>

#ifdef BENCHMARK
    #include "../timer.h"
extern Timer candidate_mpi_timer;
#endif

std::vector<std::vector<size_t>>
find_candidate_mpi(const std::string &text,
                   const std::vector<std::string> &queries);

#endif
