#include "candidate_v3_text_search.h"

#include <cstdint>
#include <cstring>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef BENCHMARK
#include "candidate_v3_text_search_benchmark.h"
#endif

namespace {

// Cross-platform trailing zero count
    inline int count_trailing_zeros(uint64_t x) {
#ifdef _MSC_VER
        unsigned long index;
        return _BitScanForward64(&index, x) ? static_cast<int>(index) : 64;
#else
        return __builtin_ctzll(x);
#endif
    }

    void find_candidates(uint64_t **mask, unsigned long *mask_words,
                         const std::string &text, const std::string &query) {
        const auto text_length = text.length();
        const auto query_length = query.length();

#ifdef BENCHMARK
        candidate_v3_timer.start_sequential_part(0, "allocate bitmask");
#endif
        *mask_words = (text_length + 63) / 64;
        *mask = new uint64_t[*mask_words]();
#ifdef BENCHMARK
        candidate_v3_timer.stop_sequential_part(0);
#endif

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
find_candidate_v3(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices(queries.size());

    for (int i = 0; i < queries.size(); ++i) {
        const auto &query = queries[i];

        uint64_t *mask;
        unsigned long mask_words;

#ifdef BENCHMARK
        candidate_v3_timer.start_sequential_part(1, "find candidates");
#endif

        find_candidates(&mask, &mask_words, text, query);

#ifdef BENCHMARK
        candidate_v3_timer.stop_sequential_part(1);
#endif

        for (int word = 0; word < mask_words; ++word) {
            uint64_t w = mask[word];

#ifdef BENCHMARK
            candidate_v3_timer.start_sequential_part(2, "test candidates");
#endif

            while (w != 0) {
                int index = word * 64 + count_trailing_zeros(w);

                if (test_candidate(index, text, query)) {
                    indices[i].push_back(index);
                }

                w &= (w - 1);
            }

#ifdef BENCHMARK
            candidate_v3_timer.stop_sequential_part(2);
#endif
        }

        delete[] mask;
    }

    return indices;
}
