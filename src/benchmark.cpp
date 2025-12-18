#include "candidate/candidate_text_search_benchmark.h"
#include "cxxopts.hpp"
#include "hash/hash_text_search_benchmark.h"
#include "std/std_text_search_benchmark.h"
#include "util.h"

#include <iostream>

int main(const int argc, char **argv) {
    cxxopts::Options options("text-search-benchmark",
                             "Search for words in big texts and benchmark it");

    options.add_options()("q,query", "queries",
                          cxxopts::value<std::vector<std::string>>())(
        "d,directory", "directories to search in",
        cxxopts::value<std::vector<std::string>>());

    const auto result = options.parse(argc, argv);

    if (!result.count("query")) {
        std::cerr << "No queries passed." << std::endl;
        return 1;
    }

    auto queries = result["query"].as<std::vector<std::string>>();

    if (!result.count("directory")) {
        std::cerr << "Please specify at least one directory.";

        return 1;
    }

    std::vector<std::string> texts;

    auto directories = result["directory"].as<std::vector<std::string>>();

    for (auto &directory : directories) {
        auto contents = read_directory(directory);

        for (auto &[file, content] : contents) {
            std::cout << "Read file " << file << std::endl;

            texts.push_back(content);
        }
    }

    std::string total;

    for (const auto &text : texts) {
        total += text;
    }

    std::cout << "Assembled all texts to one of size " << total.length()
              << std::endl;

    benchmark_candidate(total, queries);
    benchmark_std(total, queries);
    benchmark_hash(total, queries);

    sequential_timer.print();
    std_timer.print();
    hash_timer.print();
}