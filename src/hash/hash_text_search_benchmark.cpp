#include "hash_text_search_benchmark.h"

#include "hash_text_search.h"

#include <iostream>

Timer hash_timer = Timer(std::string("hash"));

std::vector<std::vector<int>>
benchmark_hash(const std::string &text,
               const std::vector<std::string> &queries) {
    hash_timer.start_total();

    auto results = find_hash(text, queries);

    hash_timer.stop_total();

    std::cout << "Finished hash text search." << std::endl;

    hash_timer.print();

    return results;
}
