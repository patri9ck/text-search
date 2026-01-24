#include "hash_openmp_text_search.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <omp.h>
#include <string>
#include <unordered_map>
#include <vector>

#define PRIME 1000003

#ifdef BENCHMARK
Timer hash_openmp_timer = Timer(std::string("hash_openmp"));
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

struct LengthBlock {
        size_t length;
        size_t start_idx;
        size_t count;
};

} // namespace

std::vector<std::vector<size_t>>
find_hash_openmp(const std::string &text,
                 const std::vector<std::string> &queries) {
    const int max_threads = omp_get_max_threads();

    omp_set_num_threads(max_threads);

    std::vector<std::vector<size_t>> indices(queries.size());

    const size_t text_length = text.length();
    const size_t query_amount = queries.size();

    std::vector<size_t> query_indices(query_amount);
    std::iota(query_indices.begin(), query_indices.end(), 0);

    std::ranges::sort(query_indices, [&](size_t a, size_t b) {
        return queries[a].length() < queries[b].length();
    });

    std::vector<LengthBlock> blocks;

    for (size_t i = 0; i < query_amount;) {
        const size_t query_length = queries[query_indices[i]].length();
        const size_t start = i;

        while (i < query_amount &&
               queries[query_indices[i]].length() == query_length) {
            ++i;
        }

        if (query_length > 0 && query_length <= text_length) {
            blocks.push_back({query_length, start, i - start});
        }
    }

#pragma omp parallel for schedule(dynamic) default(none) shared(blocks, query_indices, queries, text, text_length, indices)
    for (const auto &[length, start_idx, count] : blocks) {
        std::unordered_map<uint64_t, std::vector<size_t>> hash_to_queries;

        for (size_t i = 0; i < count; ++i) {
            const size_t q_idx = query_indices[start_idx + i];
            const uint64_t h = hash_string(queries[q_idx].data(), length);
            hash_to_queries[h].push_back(q_idx);
        }

        const uint64_t power = compute_power(length);

        uint64_t window_hash = hash_string(text.data(), length);

        for (size_t j = 0; j <= text_length - length; ++j) {
            auto it = hash_to_queries.find(window_hash);

            if (it != hash_to_queries.end()) {
                for (const size_t query_index : it->second) {
                    if (std::memcmp(text.data() + j,
                                    queries[query_index].data(), length) == 0) {
                        indices[query_index].push_back(j);
                    }
                }
            }

            if (j < text_length - length) {
                window_hash -= static_cast<unsigned char>(text[j]) * power;
                window_hash = window_hash * PRIME +
                              static_cast<unsigned char>(text[j + length]);
            }
        }
    }

    return indices;
}
