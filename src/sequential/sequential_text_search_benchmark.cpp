#include "sequential_text_search_benchmark.h"

#include "sequential_text_search.h"

#include <iostream>

Timer sequential_timer = Timer(std::string("sequential"));

void benchmark_sequential(const std::string &text,
                          const std::vector<std::string> &queries) {
    sequential_timer.start_total();

    find_sequential(text, queries);

    sequential_timer.stop_total();

    std::cout << "Finished sequential text search." << std::endl;
}
