#include "candidate_opencl_v2_text_search.h"

#include <cstdint>

#ifdef BENCHMARK
    #include "candidate_opencl_v2_text_search_benchmark.h"
#endif

namespace {
void find_candidates(uint64_t **mask, unsigned long *mask_words,
                     const std::string &text, const std::string &query) {
    const auto text_length = text.length();
    const auto query_length = query.length();

    *mask_words = (text_length + 63) / 64;
    *mask = new uint64_t[*mask_words]();

    for (int i = 0; i < text_length - query_length; ++i) {
        if (text[i] == query[0]) {
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
find_candidate_opencl_v2(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices(queries.size());

    for (int i = 0; i < queries.size(); ++i) {
        const auto &query = queries[i];

        uint64_t *mask;
        unsigned long mask_words;

        find_candidates(&mask, &mask_words, text, query);

        for (int word = 0; word < mask_words; ++word) {
            uint64_t w = mask[word];

            while (w != 0) {
                int index = word * 64 + std::countr_zero(w);

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
