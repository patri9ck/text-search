#include "cxxopts.hpp"
#include "util.h"
#include "sequential_text_search.h"
#include "sequential_text_search_std.h"
#include "sequential_text_search_hash.h"

#include <iostream>
#include <ranges>

int main(const int argc, char **argv) {
    cxxopts::Options options("text-search", "Search for words in big texts");

    options.add_options()
        ("i,implementation", "implementation: sequential, std, parallel", cxxopts::value<std::string>()->default_value("sequential"))
        ("f,file", "files to search in", cxxopts::value<std::vector<std::string>>())
        ("d,directory", "directories to search in", cxxopts::value<std::vector<std::string>>())
        ("q,query", "queries", cxxopts::value<std::vector<std::string>>())
        ("h,help", "Print help");

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (!result.count("query")) {
        std::cerr << "No queries passed." << std::endl;
        return 1;
    }

    auto queries = result["query"].as<std::vector<std::string>>();

    std::map<std::string, std::string> texts;

    if (result.count("file")) {
        auto files = result["file"].as<std::vector<std::string>>();

        for (auto &file : files) {
            auto content = read_file(file);

            if (content) {
                texts[file] = *content;
            } else {
                std::cerr << "Warning: File couldn't be read: " << file << std::endl;
            }
        }
    }

    if (result.count("directory")) {
        auto directories = result["directory"].as<std::vector<std::string>>();

        for (auto &directory : directories) {
            auto contents = read_directory(directory);

            texts.insert(contents.begin(), contents.end());
        }
    }

    if (texts.empty()) {
        std::cerr << "No texts are remaining." << std::endl;
        return 1;
    }

    std::cout << "Files to look through: " << std::endl;

    for (const auto &file : texts | std::views::keys) {
        std::cout << file << std::endl;
    }

    std::string impl = result["implementation"].as<std::string>();
    std::cout << "searching with: " << impl << std::endl;

    std::vector<std::vector<int>> matches;

    for (const auto& [filename, content] : texts) {
        if (impl == "sequential") {
            matches = find_sequential(content, queries);
        } else if (impl == "std") {
            matches = find_sequential_std(content, queries);
        } else if (impl == "hash")
            matches = find_sequential_hash(content, queries);

        if (matches.size() != queries.size()) {
            std::cerr << "Warning: unexpected result size for file " << filename << std::endl;
            continue;
        }

        std::cout << "File: " << filename << std::endl;
        for (size_t q = 0; q < queries.size(); ++q) {
            std::cout << "  Query: " << queries[q] << "\n";
            for (auto pos : matches[q]) {
                std::cout << "    Found at position: " << pos << "\n";
            }
        }
    }

}