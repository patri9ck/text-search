#include "util.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <mpi.h>

std::optional<std::string> read_file(const std::string &path, bool silent) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        if (!silent) {
            std::cerr << "Failed to open file: " << path << std::endl;
        }
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << f.rdbuf();

    if (f.fail() && !f.eof()) {
        if (!silent) {
            std::cerr << "Error while reading file: " << path << std::endl;
        }
        return std::nullopt;
    }

    return buffer.str();
}

std::map<std::string, std::string> read_directory(const std::string &path,
                                                  bool silent) {
    std::map<std::string, std::string> contents;

    try {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                auto content = read_file(entry.path().string());

                if (content) {
                    contents[entry.path().string()] = *content;
                }
            } else if (!silent) {
                std::cout << entry.path()
                          << " is not a regular file, skipping..." << std::endl;
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error while reading directory content: " << e.what()
                  << std::endl;
    }

    return contents;
}

MPIManager::MPIManager(int argc, char **argv) {
    int is_init = 0;
    MPI_Initialized(&is_init);
    if (!is_init) {
        MPI_Init(&argc, &argv);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // deaktiviere cou für alle außer Rank 0
    if (!is_master()) {
        std::cout.setstate(std::ios_base::failbit);
        std::cerr.setstate(std::ios_base::failbit);
    }
}

MPIManager::~MPIManager() {
    int is_finalized = 0;
    MPI_Finalized(&is_finalized);
    if (!is_finalized) {
        MPI_Finalize();
    }
}