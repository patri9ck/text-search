#include "candidate_v4_text_search.h"

#include <cstdint>

#ifdef BENCHMARK
    #include "candidate_v4_text_search_benchmark.h"
#endif

namespace {
void find_candidates(uint64_t *mask, int mask_words, const std::string &text,
                     const std::vector<std::string> &queries) {
    for (int i = 0; i < text.length(); ++i) {
        for (int j = 0; j < queries.size(); ++j) {
            if (text[i] == queries[j][0]) {
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
find_candidate_v4(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices;

#ifdef BENCHMARK
    candidate_v4_timer.start_sequential_part(0, "allocate bitmask");
#endif

    int mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

#ifdef BENCHMARK
    candidate_v4_timer.stop_sequential_part(0);
#endif

#ifdef BENCHMARK
    candidate_v4_timer.start_sequential_part(1, "find candidates");
#endif

    find_candidates(mask, mask_words, text, queries);

#ifdef BENCHMARK
    candidate_v4_timer.stop_sequential_part(1);
#endif

#ifdef BENCHMARK
    candidate_v4_timer.start_sequential_part(2, "test candidates");
#endif

    for (int i = 0; i < queries.size(); ++i) {
        indices.emplace_back();

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

#ifdef BENCHMARK
    candidate_v4_timer.stop_sequential_part(2);
#endif

    delete[] mask;

    return indices;
}