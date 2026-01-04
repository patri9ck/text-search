#include "candidate_v2_text_search.h"

#include <cstdint>
#include <cstring>

#ifdef BENCHMARK
Timer candidate_v2_timer = Timer(std::string("candidate_v2"));
#endif

namespace {
void find_candidates(size_t **results, size_t *results_size,
                     const std::string &text, const std::string &query) {
    const auto text_length = text.length();
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

#ifdef BENCHMARK
    candidate_v2_timer.start_sequential_part(0, "allocate array");
#endif
    *results = new size_t[text.size()];
    *results_size = 0;
#ifdef BENCHMARK
    candidate_v2_timer.stop_sequential_part(0);
#endif

    for (size_t i = 0; i < text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
            (*results)[*results_size] = i;
            *results_size = *results_size + 1;
        }
    }
}

bool test_candidate(const size_t index, const std::string &text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}

} // namespace

std::vector<std::vector<size_t>>
find_candidate_v2(const std::string &text,
                  const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    for (int i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        size_t *results;
        size_t results_size;

#ifdef BENCHMARK
        candidate_v2_timer.start_sequential_part(1, "find candidates");
#endif

        find_candidates(&results, &results_size, text, query);

#ifdef BENCHMARK
        candidate_v2_timer.stop_sequential_part(1);
#endif

        for (size_t j = 0; j < results_size; ++j) {
            auto result = results[j];

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