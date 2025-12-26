#include "candidate_v4_text_search_benchmark.h"

#include "candidate_v4_text_search.h"

#include <iostream>

Timer candidate_v4_timer = Timer(std::string("candidate_v4"));

std::vector<std::vector<int>>
benchmark_candidate_v4(const std::string &text,
                       const std::vector<std::string> &queries) {
    candidate_v4_timer.start_total();

    auto results = find_candidate_v4(text, queries);

    candidate_v4_timer.stop_total();

    std::cout << "Finished candidate_v4 text search." << std::endl;

    candidate_v4_timer.print();

    return results;
}
