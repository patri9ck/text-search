#include "candidate_opencl_v2_text_search.h"

#include <cstdint>
#include <cstring>

#ifdef BENCHMARK
Timer candidate_opencl_v2_timer = Timer(std::string("candidate_opencl_v2"));
#endif

namespace {
void find_candidates(uint64_t **mask, unsigned long *mask_words,
                     const std::string &text, const std::string &query) {
    const auto text_length = text.length();
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    *mask_words = (text_length - query_length + 64) / 64;
    *mask = new uint64_t[*mask_words]();

    for (size_t i = 0; i <= text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
            (*mask)[i >> 6] |= static_cast<uint64_t>(1) << (i & 63);
        }
    }
}

bool test_candidate(const int index, const std::string &text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}

} // namespace

std::vector<std::vector<size_t>>
find_candidate_opencl_v2(const std::string &text,
                         const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    for (size_t i = 0; i < queries.size(); ++i) {
        const auto &query = queries[i];

        uint64_t *mask;
        unsigned long mask_words;

        find_candidates(&mask, &mask_words, text, query);

        for (unsigned long word = 0; word < mask_words; ++word) {
            auto w = mask[word];

            if (w == 0) {
                continue;
            }

            while (w != 0) {
                auto index = word * 64 + std::countr_zero(w);

                if (test_candidate(index, text, query)) {
                    indices[i].push_back(index);
                }

                w &= w - 1;
            }
        }

        delete[] mask;
    }

    return indices;
}
