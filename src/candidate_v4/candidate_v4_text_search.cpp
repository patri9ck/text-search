#include "candidate_v4_text_search.h"

#include <cstdint>
#include <cstring>

#ifdef BENCHMARK
Timer candidate_v4_timer = Timer(std::string("candidate_v4"));
#endif

namespace {
void find_candidates(uint64_t *mask, unsigned long mask_words,
                     const std::string &text,
                     const std::vector<std::string> &queries) {
    for (size_t i = 0; i < text.length(); ++i) {
        for (size_t j = 0; j < queries.size(); ++j) {
            auto &query = queries[j];
            auto query_length = query.size();

            const auto mid = query_length >> 1;
            const auto end = query_length - 1;

            if (text.length() - i >= query_length && text[i] == query[0] &&
                text[i + mid] == query[mid] && text[i + end] == query[end]) {
                mask[j * mask_words + (i >> 6)] |= static_cast<uint64_t>(1)
                                                   << (i & 63);
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
find_candidate_v4(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    unsigned long mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

    find_candidates(mask, mask_words, text, queries);

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        for (unsigned long word = 0; word < mask_words; ++word) {
            auto w = mask[i * mask_words + word];

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