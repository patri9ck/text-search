#include "candidate_v3_text_search.h"

#include <cstdint>
#include <cstring>

#ifdef BENCHMARK
Timer candidate_v3_timer = Timer(std::string("candidate_v3"));
#endif

namespace {
void find_candidates(const size_t text_length, uint64_t *mask,
                     const std::string &text, const std::string &query) {
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    for (size_t i = 0; i <= text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
            mask[i >> 6] |= static_cast<uint64_t>(1) << (i & 63);
        }
    }
}

bool test_candidate(const size_t index, const std::string &text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}

} // namespace

std::vector<std::vector<size_t>>
find_candidate_v3(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    const auto text_length = text.length();

    const unsigned long mask_words = (text_length + 63) / 64;
    auto *mask = new uint64_t[mask_words]();

    for (size_t i = 0; i < queries.size(); ++i) {
        const auto &query = queries[i];

        find_candidates(text_length, mask, text, query);

        for (unsigned long word = 0; word < mask_words; ++word) {
            auto w = mask[word];

            while (w != 0) {
                auto index = word * 64 + std::countr_zero(w);

                if (test_candidate(index, text, query)) {
                    indices[i].push_back(index);
                }

                w &= w - 1;
            }
        }

        std::memset(mask, 0, mask_words * sizeof(uint64_t));
    }

    delete[] mask;

    return indices;
}
