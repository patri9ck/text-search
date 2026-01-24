#include "hash_v2_text_search.h"

#include <algorithm>
#include <cstring>
#include <unordered_map>

#define PRIME 1000003

#ifdef BENCHMARK
Timer hash_v2_timer = Timer(std::string("hash_v2"));
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
find_hash_v2(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    const size_t text_length = text.length();

    std::unordered_map<size_t, std::vector<size_t>> length_groups;

    for (size_t i = 0; i < queries.size(); ++i) {
        length_groups[queries[i].length()].push_back(i);
    }

    for (auto const &[query_length, query_indices] : length_groups) {
        if (query_length == 0 || query_length > text_length) {
            continue;
        }

        std::unordered_map<uint64_t, std::vector<size_t>> hash_table;

        for (size_t q : query_indices) {
            uint64_t h = hash_string(queries[q].data(), query_length);
            hash_table[h].push_back(q);
        }

        const uint64_t power = compute_power(query_length);

        uint64_t window_hash = hash_string(text.data(), query_length);

        for (size_t j = 0; j <= text_length - query_length; ++j) {
            auto it = hash_table.find(window_hash);

            if (it != hash_table.end()) {
                for (const auto q : it->second) {
                    if (std::memcmp(text.data() + j, queries[q].data(),
                                    query_length) == 0) {
                        indices[q].push_back(j);
                    }
                }
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
