#include "candidate_v2_text_search.h"

#include <cstdint>
#include <cstring>

#ifdef BENCHMARK
    #include "candidate_v2_text_search_benchmark.h"
#endif

namespace {
void find_candidates(int **results, int *results_size, const std::string &text,
                     const std::string &query) {
    const auto text_length = text.length();
    const auto query_length = query.length();

#ifdef BENCHMARK
    candidate_v2_timer.start_sequential_part(0, "allocate array");
#endif
    *results = new int[text.size()];
    *results_size = 0;
#ifdef BENCHMARK
    candidate_v2_timer.stop_sequential_part(0);
#endif

    for (int i = 0; i < text_length - query_length; ++i) {
        if (text[i] == query[0]) {
            (*results)[*results_size] = i;
            *results_size = *results_size + 1;
        }
    }
}

bool test_candidate(const int index, const std::string &text,
                    const std::string &query) {
    const auto query_length = query.length();

    for (size_t i = 0; i < query_length; ++i) {
        if (query[i] != text[i + index]) {
            return false;
        }
    }

    return true;
}

} // namespace

std::vector<std::vector<int>>
find_candidate_v2(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices(queries.size());

    for (int i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        int *results;
        int results_size;

#ifdef BENCHMARK
        candidate_v2_timer.start_sequential_part(1, "find candidates");
#endif

        find_candidates(&results, &results_size, text, query);

#ifdef BENCHMARK
        candidate_v2_timer.stop_sequential_part(1);
#endif

        for (int j = 0; j < results_size; ++j) {
            int result = results[j];

#ifdef BENCHMARK
            candidate_v2_timer.start_sequential_part(2, "test candidates");
#endif

            if (test_candidate(result, text, query)) {
                indices[i].push_back(result);
            }

#ifdef BENCHMARK
            candidate_v2_timer.stop_sequential_part(2);
#endif
        }

        delete[] results;
    }

    return indices;
}