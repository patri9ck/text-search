#include "candidate_v1_text_search_benchmark.h"

#include "candidate_v1_text_search.h"

#include <iostream>

Timer candidate_v1_timer = Timer(std::string("candidate_v1"));

std::vector<std::vector<int>>
benchmark_candidate_v1(const std::string &text,
                       const std::vector<std::string> &queries) {
    candidate_v1_timer.start_total();

    auto results = find_candidate_v1(text, queries);

    candidate_v1_timer.stop_total();

    std::cout << "Finished candidate_v1 text search." << std::endl;

    candidate_v1_timer.print();

    return results;
}
