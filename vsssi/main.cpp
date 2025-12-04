#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <chrono>

std::optional<std::string> read_file(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << f.rdbuf();

    if (f.fail() && !f.eof()) {
        std::cerr << "Error while reading file: " << path << std::endl;
        return std::nullopt;
    }

    return buffer.str();
}

void find_candidates(uint64_t **mask, size_t *mask_words, const char *text, const size_t text_len, const char *query, const size_t query_len) {
    *mask_words = (text_len + 63) / 64;
    *mask = new uint64_t[*mask_words];
    std::memset(*mask, 0, *mask_words * sizeof(uint64_t));

    for (int i = 0; i <= text_len - query_len; ++i) {

        if (text[i] == query[0] && text[i + query_len - 1] == query[query_len - 1]) {
            (*mask)[i >> 6] |= static_cast<uint64_t>(1) << (i & 63);
        }
    }
}

bool test_specific_candidate(const size_t index, const char *text, const char *query, const size_t query_len) {
    for (size_t i = 0; i < query_len; ++i) {
        if (query[i] != text[i + index]) {
            return false;
        }
    }

    return true;
}

std::vector<size_t> test_candidates(const uint64_t *mask, const size_t mask_words, const char *text, const char *query, const size_t query_len) {
    std::vector<size_t> results;

    for (size_t word = 0; word < mask_words; ++word) {
        uint64_t w = mask[word];

        while (w != 0) {
            size_t index = word * 64 + __builtin_ctzll(w);

            if (test_specific_candidate(index, text, query, query_len)) {
                results.push_back(index);
            }

            w &= (w - 1);
        }
    }

    return results;
}

std::vector<size_t> find_occurrences_std(const std::string& text, const std::string& query) {
    std::vector<size_t> results;

    size_t pos = text.find(query);
    while (pos != std::string::npos) {
        results.push_back(pos);

        pos = text.find(query, pos + 1);
    }

    return results;
}


int main(const int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: sequential-text-search <file> <query>" << std::endl;
        return 1;
    }

    const auto text_std = read_file(argv[1]);

    if (!text_std) {
        return 1;
    }

    uint64_t *mask;
    size_t mask_words;

    const char *text = text_std->c_str();
    const size_t text_len = strlen(text);
    const char *query = argv[2];
    const size_t query_len = strlen(query);

    auto t1 = std::chrono::high_resolution_clock::now();

    find_candidates(&mask, &mask_words, text, text_len, query, query_len);

    const std::vector<size_t> results = test_candidates(mask, mask_words, text, query, query_len);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Results: " << std::endl;

    for (const size_t result : results) {
        std::cout << result << std::endl;
    }

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << ms << " ms\n";

    t1 = std::chrono::high_resolution_clock::now();

    const std::vector<size_t> std_results = find_occurrences_std(std::string(text), std::string(query));

    t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Standard library results: " << std::endl;

    for (const size_t result : std_results) {
        std::cout << result << std::endl;
    }

    ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << ms << " ms\n";

    return 0;
}
