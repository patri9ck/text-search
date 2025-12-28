#include "candidate_openmp_v1_text_search.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <omp.h>

#ifdef BENCHMARK
    #include "candidate_openmp_v1_text_search_benchmark.h"
#endif

namespace {
void find_candidates(uint64_t **mask, unsigned long *mask_words,
                     const std::string &text, const std::string &query) {
    const auto text_length = text.length();
    const auto query_length = query.length();

    *mask_words = (text_length + 63) / 64;
    *mask = new uint64_t[*mask_words]();

    for (int i = 0; i <= text_length - query_length; ++i) {
        if (text[i] == query[0] &&
            text[i + query_length - 1] == query[query_length - 1]) {
            (*mask)[i >> 6] |= static_cast<uint64_t>(1) << (i & 63);
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
find_candidate_openmp_v1(const std::string &text,
                         const std::vector<std::string> &queries) {
    const auto max_threads = omp_get_max_threads();

    omp_set_num_threads(max_threads);

#ifdef BENCHMARK
    std::cout << "Using " << max_threads << " threads." << std::endl;
#endif

    std::vector<std::vector<int>> indices(queries.size());

#pragma omp parallel for default(none) shared(queries, indices, text)
    for (int i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        uint64_t *mask;
        unsigned long mask_words;

        find_candidates(&mask, &mask_words, text, query);

        for (int word = 0; word < mask_words; ++word) {
            uint64_t w = mask[word];

            while (w != 0) {
                int index = word * 64 + __builtin_ctzll(w);

                if (test_candidate(index, text, query)) {
                    indices[i].push_back(index);
                }

                w &= (w - 1);
            }
        }

        delete[] mask;
    }

    return indices;
}