#include "candidate_openmp_v2_text_search.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <omp.h>

#ifdef BENCHMARK
Timer candidate_openmp_v2_timer = Timer(std::string("candidate_openmp_v2"));
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

        const auto mid = query_length >> 1;
        const auto end = query_length - 1;

        for (int j = 0; j <= text_length - query_length; ++j) {
            if (text[j] == query[0] && text[j + mid] == query[mid] &&
                text[j + end] == query[end]) {
#pragma omp atomic
                mask[i * mask_words + (j >> 6)] |= static_cast<uint64_t>(1)
                                                   << (j & 63);
            }
        }
    }
}

bool test_candidate(const size_t index, const std::string &text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}
} // namespace

std::vector<std::vector<size_t>>
find_candidate_openmp_v2(const std::string &text,
                         const std::vector<std::string> &queries) {
    const int max_threads = omp_get_max_threads();

    omp_set_num_threads(max_threads);

#ifdef BENCHMARK
    std::cout << "Using " << max_threads << " threads." << std::endl;
#endif

    std::vector<std::vector<size_t>> indices(queries.size());

    const auto mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

    find_candidates(mask, mask_words, text, queries);

#pragma omp parallel for default(none)                                         \
    shared(queries, mask_words, mask, text, indices)
    for (long long i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        for (unsigned long word = 0; word < mask_words; ++word) {
            uint64_t w = mask[i * mask_words + word];

            if (w == 0) {
                continue;
            }

            while (w != 0) {
                auto index = word * 64 + std::countr_zero(w);

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