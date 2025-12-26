#include "candidate_v3_text_search_benchmark.h"

#include "candidate_v3_text_search.h"

#include <iostream>

Timer candidate_v3_timer = Timer(std::string("candidate_v3"));

std::vector<std::vector<int>>
benchmark_candidate_v3(const std::string &text,
                       const std::vector<std::string> &queries) {
    candidate_v3_timer.start_total();

    auto results = find_candidate_v3(text, queries);

    candidate_v3_timer.stop_total();

    std::cout << "Finished candidate_v3 text search." << std::endl;

    candidate_v3_timer.print();

    return results;
}
