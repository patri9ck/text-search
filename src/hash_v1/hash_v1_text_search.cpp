#include "hash_v1_text_search.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#define PRIME 1000003

#ifdef BENCHMARK
Timer hash_v1_timer = Timer(std::string("hash_v1"));
#endif

namespace {

uint64_t compute_power(const size_t length) {
    uint64_t p = 1;

    for (size_t i = 1; i < length; ++i) {
        p *= PRIME;
    }

    return p;
}

uint64_t hash_string(const char *s, const size_t length) {
    uint64_t h = 0;

    for (size_t i = 0; i < length; ++i) {
        h = h * PRIME + static_cast<unsigned char>(s[i]);
    }

    return h;
}

} // namespace

std::vector<std::vector<size_t>>
find_hash_v1(const std::string &text, const std::vector<std::string> &queries) {

    std::vector<std::vector<size_t>> indices(queries.size());
    const size_t text_length = text.length();

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];
        const size_t query_length = query.length();

        if (query_length == 0 || query_length > text_length) {
            continue;
        }

        const uint64_t query_hash = hash_string(query.data(), query_length);

        const uint64_t power = compute_power(query_length);

        uint64_t window_hash = hash_string(text.data(), query_length);

        for (size_t j = 0; j <= text_length - query_length; ++j) {
            if (window_hash == query_hash &&
                std::memcmp(text.data() + j, query.data(), query_length) == 0) {
                indices[i].push_back(j);
            }

            if (j < text_length - query_length) {
                window_hash -= static_cast<unsigned char>(text[j]) * power;

                window_hash = window_hash * PRIME + static_cast<unsigned char>(
                                                        text[j + query_length]);
            }
        }
    }

    return indices;
}
