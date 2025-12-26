#include "candidate_text_search_benchmark.h"

#include "candidate_text_search.h"

#include <iostream>

Timer candidate_timer = Timer(std::string("candidate"));

std::vector<std::vector<int>>
benchmark_candidate(const std::string &text,
                    const std::vector<std::string> &queries) {
    candidate_timer.start_total();

    auto results = find_candidate(text, queries);

    candidate_timer.stop_total();

    std::cout << "Finished candidate text search." << std::endl;

    candidate_timer.print();

    return results;
}
