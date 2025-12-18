#include "cxxopts.hpp"
#include "hash/hash_text_search.h"
#include "sequential/sequential_text_search.h"
#include "std/std_text_search.h"
#include "util.h"

#include <iostream>

int main(const int argc, char **argv) {
    cxxopts::Options options("text-search", "Search for words in big texts");

    options.add_options()(
        "i,implementation", "implementation: sequential, std, hash",
        cxxopts::value<std::string>()->default_value("sequential"))(
        "f,file", "files to search in",
        cxxopts::value<std::vector<std::string>>())(
        "d,directory", "directories to search in",
        cxxopts::value<std::vector<std::string>>())(
        "q,query", "queries",
        cxxopts::value<std::vector<std::string>>())("h,help", "Print help");

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (!result.count("query")) {
        std::cerr << "No queries passed." << std::endl;
        return 1;
    }

    std::string implementation = result["implementation"].as<std::string>();

    std::vector<std::vector<int>> (*find)(const std::string &, const std::vector<std::string> &);

    if (implementation == "sequential") {
        find = find_sequential;
    } else if (implementation == "hash") {
        find = find_hash;
    } else if (implementation == "std") {
        find = find_std;
    } else {
        std::cerr << "Unknown implementation." << std::endl;
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

    std::string total;

    std::cout << "Files to look through: " << std::endl;

    for (const auto &[file, text] : texts) {
        std::cout << file << std::endl;

        total += text;
    }

    std::cout << "Searching with " << implementation << " through an assembled text of size " << total.length() << "." << std::endl;

    auto matches = find(total, queries);

    for (int i = 0; i < queries.size(); ++i) {
        const auto& query = queries[i];

        std::cout << "Query " << query << ":" << std::endl;

        auto indices = matches[i];

        for (auto it = texts.begin(); it != texts.end(); ++it) {
            std::cout << it->first << ": ";

            size_t before = 0;

            for (auto jt = texts.begin(); jt != it; ++jt) {
                before += jt->second.length();
            }

            for (auto &index : indices) {
                if (index >= before && index <= before + it->second.length()) {
                    std::cout << index << " ";
                }
            }

            std::cout << std::endl;
        }
    }
}