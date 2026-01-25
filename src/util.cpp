#include "util.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

std::optional<std::string> read_file(const std::string &path,
                                     const bool silent) {
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
                                                  const bool silent) {
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

void run_mpi(const int err) {
    if (err != MPI_SUCCESS) {
        char err_string[MPI_MAX_ERROR_STRING];
        int length;

        MPI_Error_string(err, err_string, &length);

        std::stringstream ss;

        ss << "mpi error: " << err << std::endl;

        const auto s = ss.str();

        throw std::runtime_error(s);
    }
}
