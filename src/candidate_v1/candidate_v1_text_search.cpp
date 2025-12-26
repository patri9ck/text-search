#include "candidate_v1_text_search.h"

#ifdef BENCHMARK
    #include "candidate_v1_text_search_benchmark.h"
#endif

namespace {
void find_candidates(std::vector<int> &results, const std::string &text,
                     const std::string &query) {
    const int text_length = text.length();
    const int query_length = query.length();

    for (int i = 0; i <= text_length - query_length; ++i) {
        if (text[i] == query[0]) {
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
find_candidate_v1(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices;

    for (int i = 0; i < queries.size(); ++i) {
        indices.emplace_back();

        const std::string &query = queries[i];

        std::vector<int> results;

#ifdef BENCHMARK
        candidate_v1_timer.start_sequential_part(1, "find candidates");
#endif

        find_candidates(results, text, query);

#ifdef BENCHMARK
        candidate_v1_timer.stop_sequential_part(1);
#endif

        for (auto &result : results) {
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