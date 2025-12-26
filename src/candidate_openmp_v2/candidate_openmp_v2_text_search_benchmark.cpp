#include "candidate_openmp_v2_text_search_benchmark.h"

#include "candidate_openmp_v2_text_search.h"

#include <iostream>

Timer candidate_openmp_v2_timer = Timer(std::string("candidate_openmp_v2"));

std::vector<std::vector<int>>
benchmark_candidate_openmp_v2(const std::string &text,
                       const std::vector<std::string> &queries) {
    candidate_openmp_v2_timer.start_total();

    auto results = find_candidate_openmp_v2(text, queries);

    candidate_openmp_v2_timer.stop_total();

    std::cout << "Finished candidate_openmp_v2 text search." << std::endl;

    candidate_openmp_v2_timer.print();

    return results;
}
