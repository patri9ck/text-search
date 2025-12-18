#include "candidate_text_search_benchmark.h"

#include "candidate_text_search.h"

#include <iostream>

Timer sequential_timer = Timer(std::string("sequential"));

void benchmark_candidate(const std::string &text,
                         const std::vector<std::string> &queries) {
    sequential_timer.start_total();

    find_candidate(text, queries);

    sequential_timer.stop_total();

    std::cout << "Finished sequential text search." << std::endl;
}
