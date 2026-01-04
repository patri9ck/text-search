#include "hash_text_search.h"

#include <cstdint>
#include <string>
#include <vector>

#define PRIME 131

#ifdef BENCHMARK
Timer hash_timer = Timer(std::string("hash"));
#endif

namespace {

uint64_t compute_power(size_t length) {
    uint64_t p = 1;
    for (size_t i = 1; i < length; ++i) {
        p *= PRIME;
    }
    return p;
}

} // namespace

std::vector<std::vector<size_t>>
find_hash(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    auto text_length = text.length();

    for (size_t i = 0; i < queries.size(); ++i) {
        auto const &query = queries[i];

        auto m = query.length();

        uint64_t query_hash = 0;

        for (size_t j = 0; j < m; ++j) {
            query_hash = query_hash * PRIME + query[j];
        }

        auto power = compute_power(m);

        uint64_t window_hash = 0;
        for (size_t j = 0; j < m; ++j) {
            window_hash = window_hash * PRIME + text[j];
        }

        bool match = true;

        if (window_hash == query_hash) {
            for (size_t j = 0; j < m; ++j) {
                if (text[j] != query[j]) {
                    match = false;

                    break;
                }
            }

            if (match) {
                indices[i].push_back(0);
            }
        }

        for (size_t j = 1; j <= text_length - m; ++j) {
            match = true;

            window_hash -= static_cast<uint64_t>(text[j - 1]) * power;
            window_hash *= PRIME;
            window_hash += text[j + m - 1];

            if (window_hash == query_hash) {
                for (size_t k = 0; k < m; ++k) {
                    if (text[k + j] != query[k]) {
                        match = false;

                        break;
                    }
                }

                if (match) {
                    indices[i].push_back(j);
                }
            }
        }
    }

    return indices;
}
