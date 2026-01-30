#include "candidate_mpi/candidate_mpi_text_search.h"
#include "candidate_opencl_v2/candidate_opencl_v2_text_search.h"
#include "candidate_opencl_v3/candidate_opencl_v3_text_search.h"
#include "cxxopts.hpp"
#include "std/std_text_search.h"
#include "std_openmp/std_openmp_text_search.h"
#include "timer.h"
#include "util.h"

#include <iostream>
#include <mpi.h>

int main(int argc, char **argv) {
    cxxopts::Options options("text-search", "Search for words in big texts");

    options.add_options()(
        "i,implementation",
        "implementation: sequential, openmp, opencl, opencl-safe, mpi",
        cxxopts::value<std::string>()->default_value("openmp"))(
        "f,file", "file to search in",
        cxxopts::value<std::vector<std::string>>())(
        "d,directory", "directory to search in",
        cxxopts::value<std::vector<std::string>>())(
        "q,query", "query",
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

    const auto implementation = result["implementation"].as<std::string>();

    std::vector<std::vector<size_t>> (*find)(const std::string &,
                                             const std::vector<std::string> &);

    std::string name;
    bool mpi = false;

    if (implementation == "sequential") {
        find = find_std;
        name = "std";
    } else if (implementation == "openmp") {
        find = find_std_openmp;
        name = "std_openmp";
    } else if (implementation == "mpi") {
        find = find_candidate_mpi;
        name = "candidate_mpi";
        mpi = true;
    } else if (implementation == "opencl") {
        find = find_candidate_opencl_v3;
        name = "candidate_opencl_v3";
    } else if (implementation == "opencl-safe") {
        find = find_candidate_opencl_v2;
        name = "candidate_opencl_v2";
    } else {
        std::cerr << "Unknown implementation." << std::endl;
        return 1;
    }

    if (mpi) {
        MPI_Init(&argc, &argv);

        MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

        int rank;
        run_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

        if (rank != 0) {
            find(std::string(), std::vector<std::string>());

            return 0;
        }
    }

    const auto queries = result["query"].as<std::vector<std::string>>();

    std::map<std::string, std::string> texts;

    if (result.count("file")) {
        const auto files = result["file"].as<std::vector<std::string>>();

        for (auto &file : files) {
            const auto content = read_file(file);

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

    std::cout << "Searching with " << name
              << " through an assembled text of size " << total.length() << "."
              << std::endl;

    Timer timer("main");

    timer.start_total();

    auto matches = find(total, queries);

    timer.stop_total();

    std::cout << "Finished in " << timer.get_total_time() << " ms."
              << std::endl;

    for (int i = 0; i < queries.size(); ++i) {
        const auto &query = queries[i];

        std::cout << "Query " << query << ":" << std::endl;

        const auto &indices = matches[i];

        for (auto it = texts.begin(); it != texts.end(); ++it) {
            std::cout << std::endl;
            std::cout << it->first << ": ";

            size_t before = 0;

            for (auto jt = texts.begin(); jt != it; ++jt) {
                before += jt->second.length();
            }

            for (const auto &index : indices) {
                if (index >= before && index < before + it->second.length()) {
                    std::cout << (index - before) << " ";
                }
            }

            std::cout << std::endl;
        }
    }
}