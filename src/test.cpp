#include "candidate_opencl_v1/candidate_opencl_v1_text_search.h"
#include "candidate_opencl_v2/candidate_opencl_v2_text_search.h"
#include "candidate_opencl_v3/candidate_opencl_v3_text_search.h"
#include "candidate_openmp_v1/candidate_openmp_v1_text_search.h"
#include "candidate_openmp_v2/candidate_openmp_v2_text_search.h"
#include "candidate_v1/candidate_v1_text_search.h"
#include "candidate_v2/candidate_v2_text_search.h"
#include "candidate_v3/candidate_v3_text_search.h"
#include "candidate_v4/candidate_v4_text_search.h"
#include "cxxopts.hpp"
#include "hash/hash_text_search.h"
#include "hash_openmp_v1/hash_openmp_v1_text_search.h"
#include "std/std_text_search.h"
#include "util.h"

#include <algorithm>

#include <iostream>

void compare_results(std::vector<std::vector<size_t>> &expected,
                     std::vector<std::vector<size_t>> &actual,
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

    unsigned int wrong = 0;

    for (size_t i = 0; i < queries.size(); ++i) {
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

        for (unsigned long j = 0; j < max; ++j) {
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

    options.add_options()("i,implementation", "implementation",
                          cxxopts::value<std::string>())

        ("f,file", "file containing queries seperated by whitespace",
         cxxopts::value<std::vector<std::string>>())(
            "d,directory", "directory to search in",
            cxxopts::value<std::vector<std::string>>())(
            "n", "limit queries to n",
            cxxopts::value<size_t>()->default_value("0"))(
            "m", "limit files to m",
            cxxopts::value<size_t>()->default_value("0"))(
            "c,compare", "compare with std", cxxopts::value<bool>());

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    std::string implementation = result["implementation"].as<std::string>();

    std::vector<std::vector<size_t>> (*find)(const std::string &,
                                             const std::vector<std::string> &);
    Timer *timer;

    if (implementation == "candidate_v1") {
        find = find_candidate_v1;
        timer = &candidate_v1_timer;
    } else if (implementation == "candidate_v2") {
        find = find_candidate_v2;
        timer = &candidate_v2_timer;
    } else if (implementation == "candidate_v3") {
        find = find_candidate_v3;
        timer = &candidate_v3_timer;
    } else if (implementation == "candidate_v4") {
        find = find_candidate_v4;
        timer = &candidate_v4_timer;
    } else if (implementation == "hash") {
        find = find_hash;
        timer = &hash_timer;
    } else if (implementation == "candidate_openmp_v1") {
        find = find_candidate_openmp_v1;
        timer = &candidate_openmp_v1_timer;
    } else if (implementation == "candidate_openmp_v2") {
        find = find_candidate_openmp_v2;
        timer = &candidate_openmp_v2_timer;
    } else if (implementation == "candidate_opencl_v1") {
        find = find_candidate_opencl_v1;
        timer = &candidate_opencl_v1_timer;
    } else if (implementation == "candidate_opencl_v2") {
        find = find_candidate_opencl_v2;
        timer = &candidate_opencl_v2_timer;
    } else if (implementation == "candidate_opencl_v3") {
        find = find_candidate_opencl_v3;
        timer = &candidate_opencl_v3_timer;
    } else if (implementation == "hash_openmp_v1") {
        find = find_hash_openmp_v1;
        timer = &hash_openmp_v1_timer;
    } else {
        std::cerr << "Unknown implementation." << std::endl;
        return 1;
    }

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

    auto n = result["n"].as<size_t>();

    if (n < 1) {
        n = queries.size();
    } else {
        n = std::min(n, queries.size());
    }

    queries.resize(n);

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

    auto m = result["m"].as<size_t>();

    if (m < 1) {
        m = texts.size();
    } else {
        m = std::min(m, texts.size());
    }

    for (size_t i = 0; i < m; ++i) {
        total += texts[i];
    }

    std::cout << "Assembled all " << m << " texts to one of size "
              << total.length() << std::endl;

    timer->start_total();
    auto results = find(total, queries);
    timer->stop_total();

    timer->print();

    if (result["compare"].as<bool>()) {
        std_timer.start_total();
        auto std_results = find_std(total, queries);
        std_timer.stop_total();

        std_timer.print();

        compare_results(std_results, results, queries, implementation);
    }
}