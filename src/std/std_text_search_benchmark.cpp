#include "std_text_search_benchmark.h"

#include "std_text_search.h"

#include <iostream>
#include <string>

Timer std_timer = Timer(std::string("std"));

void benchmark_std(const std::string &text, const std::vector<std::string> &queries) {
    std_timer.start_total();

    find_std(text, queries);

    std_timer.stop_total();

    std::cout << "Finished std text search." << std::endl;
}
