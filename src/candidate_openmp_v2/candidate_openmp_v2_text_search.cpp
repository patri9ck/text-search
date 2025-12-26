#include "candidate_openmp_v2_text_search.h"

#include <cstdint>
#include <omp.h>

#ifdef BENCHMARK
    #include "candidate_openmp_v2_text_search_benchmark.h"
#endif

namespace {
void find_candidates(uint64_t *mask, int mask_words, const std::string &text,
                     const std::vector<std::string> &queries) {
    #pragma omp parallel
    for (int i = 0; i < text.length(); ++i) {
        for (int j = 0; j < queries.size(); ++j) {
            if (text[i] == queries[j][0]) {
                #pragma omp atomic
                mask[j * mask_words + (i >> 6)] |= uint64_t(1) << (i & 63);
            }
        }
    }
}

bool test_candidate(const int index, const std::string &text,
                    const std::string &query) {
    const int query_length = query.length();

    for (size_t i = 0; i < query_length; ++i) {
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
    std::vector<std::vector<int>> indices(queries.size());

    omp_set_num_threads(omp_get_max_threads());

    int mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

    find_candidates(mask, mask_words, text, queries);

    #pragma omp parallel for default(none) shared(queries, mask_words, mask, text, indices)
    for (int i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        for (int word = 0; word < mask_words; ++word) {
            uint64_t w = mask[i * mask_words + word];

            while (w != 0) {
                int index = word * 64 + __builtin_ctzll(w);

                if (test_candidate(index, text, query)) {
                    #pragma omp critical
                    indices[i].push_back(index);
                }

                w &= (w - 1);
            }
        }
    }

    delete[] mask;

    return indices;
}