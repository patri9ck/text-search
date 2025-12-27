#include "candidate_openmp_v1/candidate_openmp_v1_text_search_benchmark.h"
#include "candidate_openmp_v2/candidate_openmp_v2_text_search_benchmark.h"
//#include "candidate_openmp_v3/candidate_openmp_v3_text_search_benchmark.h"
#include "candidate_v1/candidate_v1_text_search_benchmark.h"
#include "candidate_v2/candidate_v2_text_search_benchmark.h"
#include "candidate_v3/candidate_v3_text_search_benchmark.h"
#include "candidate_v4/candidate_v4_text_search_benchmark.h"
#include "cxxopts.hpp"
#include "hash/hash_text_search_benchmark.h"
#include "std/std_text_search_benchmark.h"
#include "util.h"
#include <algorithm>

#include <iostream>

void compare_results(std::vector<std::vector<int>> &expected,
                     std::vector<std::vector<int>> &actual,
                     const std::vector<std::string> &queries,
                     const std::string &name) {
    if (expected.size() != queries.size()) {
        std::cout << name << ": Expected results do not match query amount."
                  << std::endl;

        return;
    }

    if (actual.size() != queries.size()) {
        std::cout << name << ": Actual results do not match query amount."
                  << std::endl;

        return;
    }

    int wrong = 0;

    for (int i = 0; i < queries.size(); ++i) {
        auto &expected_indices = expected[i];
        auto &actual_indices = actual[i];

        std::ranges::sort(expected_indices);
        std::ranges::sort(actual_indices);

        if (expected_indices == actual_indices) {
            continue;
        }

        std::cout << name << ": Results for query " << queries[i]
                  << " differ: " << std::endl;

        const auto max =
            std::max(expected_indices.size(), actual_indices.size());

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

            if (wrong == 100) {
                std::cout << "Found 100 mismatches, stopping..." << std::endl;

                return;
            }

            std::cout << name << ": Index " << j << ":" << std::endl;
            std::cout << "Expected: " << expected_index << std::endl;
            std::cout << "Actual: " << actual_index << std::endl;

            ++wrong;
        }
    }
}

int main(const int argc, char **argv) {
    cxxopts::Options options("text-search-benchmark",
                             "Search for words in big texts and benchmark it");

    options.add_options()("f,file",
                          "file containing queries seperated by whitespace",
                          cxxopts::value<std::vector<std::string>>())(
        "d,directory", "directory to search in",
        cxxopts::value<std::vector<std::string>>());

    const auto result = options.parse(argc, argv);

    if (!result.count("file")) {
        std::cerr << "No files passed." << std::endl;
        return 1;
    }

    std::vector<std::string> queries;

    auto files = result["file"].as<std::vector<std::string>>();

    for (auto &file : files) {
        auto content = read_file(file);

        if (content) {
            std::istringstream iss(*content);
            std::string word;

            while (iss >> word) {
                queries.push_back(word);
            }
        }
    }

    if (queries.empty()) {
        std::cerr << "No queries are remaining." << std::endl;
        return 1;
    }

    std::cout << "There are " << queries.size() << " queries." << std::endl;

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

    /*auto candidate_v1_results = benchmark_candidate_v1(total, queries);
    auto candidate_v2_results = benchmark_candidate_v2(total, queries);
    auto candidate_v3_results = benchmark_candidate_v3(total, queries);
    auto candidate_v4_results = benchmark_candidate_v4(total, queries);
    auto hash_results = benchmark_hash(total, queries);*/
    auto candidate_openmp_v1_results = benchmark_candidate_openmp_v1(total, queries);
    //auto candidate_openmp_v2_results = benchmark_candidate_openmp_v2(total, queries);

    /*compare_results(std_results, candidate_v1_results, queries,
    "candidate_v1"); compare_results(std_results, candidate_v2_results, queries,
    "candidate_v2"); compare_results(std_results, candidate_v3_results, queries,
    "candidate_v3"); compare_results(std_results, candidate_v4_results, queries,
    "candidate_v4"); compare_results(std_results, hash_results, queries,
    "hash");*/
    /*compare_results(std_results, candidate_openmp_v1_results, queries,
    "candidate_openmp_v1");*/
    /*compare_results(std_results, candidate_openmp_v2_results, queries,
                    "candidate_openmp_v2");*/
}