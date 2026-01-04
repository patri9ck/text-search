#include "candidate_v1_text_search.h"

#include <cstring>

#ifdef BENCHMARK
Timer candidate_v1_timer = Timer(std::string("candidate_v1"));
#endif

namespace {
void find_candidates(std::vector<size_t> &results, const std::string &text,
                     const std::string &query) {
    const auto text_length = text.length();
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    for (size_t i = 0; i < text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
#ifdef BENCHMARK
            candidate_v1_timer.start_sequential_part(0, "add to vector");
#endif
            results.push_back(i);
#ifdef BENCHMARK
            candidate_v1_timer.stop_sequential_part(0);
#endif
        }
    }
}

bool test_candidate(const size_t index, const std::string &text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}

} // namespace

std::vector<std::vector<size_t>>
find_candidate_v1(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        std::vector<size_t> results;

#ifdef BENCHMARK
        candidate_v1_timer.start_sequential_part(1, "find candidates");
#endif

        find_candidates(results, text, query);

#ifdef BENCHMARK
        candidate_v1_timer.stop_sequential_part(1);
#endif

        for (const auto &result : results) {
#ifdef BENCHMARK
            candidate_v1_timer.start_sequential_part(2, "test candidates");
#endif

            if (test_candidate(result, text, query)) {
                indices[i].push_back(result);
            }

#ifdef BENCHMARK
            candidate_v1_timer.stop_sequential_part(2);
#endif
        }
    }

    return indices;
}