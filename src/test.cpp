#include "candidate_v1/candidate_v1_text_search_benchmark.h"
#include "candidate_v2/candidate_v2_text_search_benchmark.h"
#include "candidate_v3/candidate_v3_text_search_benchmark.h"
#include "cxxopts.hpp"
#include "hash/hash_text_search_benchmark.h"
#include "std/std_text_search_benchmark.h"
#include "util.h"

#include <iostream>

void compare_results(const std::vector<std::vector<int>> &expected,
                     const std::vector<std::vector<int>> &actual,
                     const std::vector<std::string> &queries,
                     std::string name) {
    if (expected == actual) {
        std::cout << name << " returned correct results." << std::endl;

        return;
    }

    std::cout << name << " returned incorrect results:" << std::endl;
    ;

    if (expected.size() != queries.size()) {
        std::cout << "Expected results do not match query amount." << std::endl;

        return;
    }

    if (actual.size() != queries.size()) {
        std::cout << "Actual results do not match query amount." << std::endl;

        return;
    }

    for (int i = 0; i < queries.size(); ++i) {
        const auto &expected_indices = expected[i];
        const auto &actual_indices = actual[i];

        if (expected_indices == actual_indices) {
            continue;
        }

        std::cout << "Results for query " << queries[i]
                  << "differ: " << std::endl;

        int max = std::max(expected_indices.size(), actual_indices.size());

        for (int j = 0; j < max; ++j) {
            std::string expected_index;
            std::string actual_index;

            if (j < expected_indices.size()) {
                expected_index = std::to_string(expected_indices[j]);
            }

            if (j < actual_indices.size()) {
                actual_index = std::to_string(actual_indices[j]);
            }

            if (expected_index == actual_index) {
                continue;
            }

            std::cout << "Index " << j << ":" << std::endl;
            std::cout << "Expected: " << expected_index << std::endl;
            std::cout << "Actual: " << actual_index << std::endl;
        }
    }
}

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

    auto std_results = benchmark_std(total, queries);

    auto candidate_v1_results = benchmark_candidate_v1(total, queries);
    auto candidate_v2_results = benchmark_candidate_v2(total, queries);
    auto candidate_v3_results = benchmark_candidate_v3(total, queries);
    auto hash_results = benchmark_hash(total, queries);

    compare_results(std_results, candidate_v1_results, queries, "candidate_v1");
    compare_results(std_results, candidate_v2_results, queries, "candidate_v2");
    compare_results(std_results, candidate_v3_results, queries, "candidate_v3");
    compare_results(std_results, hash_results, queries, "hash");
}