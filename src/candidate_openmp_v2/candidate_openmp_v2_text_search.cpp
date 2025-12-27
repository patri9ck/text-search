#include "candidate_openmp_v2_text_search.h"

#include <cstdint>
#include <iostream>
#include <omp.h>

#ifdef BENCHMARK
    #include "candidate_openmp_v2_text_search_benchmark.h"
#endif

namespace {
void find_candidates(uint64_t *mask, unsigned long mask_words,
                     const std::string &text,
                     const std::vector<std::string> &queries) {
    auto text_length = text.length();

#pragma omp parallel for default(none)                                         \
    shared(text_length, queries, text, mask, mask_words)

    for (int i = 0; i < queries.size(); ++i) {
        const auto &query = queries[i];
        const auto query_length = query.size();
        for (int j = 0; j < text_length - query_length; ++j) {
            if (text[j] == query[0] &&
                text[j + query_length - 1] == query[query_length - 1]) {
#pragma omp atomic
                mask[i * mask_words + (j >> 6)] |= static_cast<uint64_t>(1)
                                                   << (j & 63);
            }
        }
    }
}

bool test_candidate(const int index, const std::string &text,
                    const std::string &query) {
    const auto query_length = query.length();

    for (int i = 0; i < query_length; ++i) {
        if (query[i] != text[i + index]) {
            return false;
        }
    }

    return true;
}
} // namespace

std::vector<std::vector<int>>
find_candidate_openmp_v2(const std::string &text,
                         const std::vector<std::string> &queries) {
    const int max_threads = omp_get_max_threads();

    omp_set_num_threads(max_threads);

#ifdef BENCHMARK
    std::cout << "Using " << max_threads << " threads." << std::endl;
#endif

    std::vector<std::vector<int>> indices(queries.size());

    const auto mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

    find_candidates(mask, mask_words, text, queries);

#pragma omp parallel for default(none)                                         \
    shared(queries, mask_words, mask, text, indices)
    for (int i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        for (int word = 0; word < mask_words; ++word) {
            uint64_t w = mask[i * mask_words + word];

            while (w != 0) {
                int index = word * 64 + __builtin_ctzll(w);

                if (test_candidate(index, text, query)) {
                    indices[i].push_back(index);
                }

                w &= (w - 1);
            }
        }
    }

    delete[] mask;

    return indices;
}