#include "candidate_v2_text_search.h"

#include <cstring>

#ifdef BENCHMARK
Timer candidate_v2_timer = Timer(std::string("candidate_v2"));
#endif

namespace {
void find_candidates(const size_t text_length, size_t *results,
                     size_t *results_size, const std::string &text,
                     const std::string &query) {
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    for (size_t i = 0; i < text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
            results[*results_size] = i;
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

    const auto text_length = text.length();

    auto *results = new size_t[text_length];

    for (int i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        size_t results_size = 0;

        find_candidates(text_length, results, &results_size, text, query);

        for (size_t j = 0; j < results_size; ++j) {
            auto result = results[j];

            if (test_candidate(result, text, query)) {
                indices[i].push_back(result);
            }
        }

        std::memset(results, 0, text_length * sizeof(size_t));
    }

    delete[] results;

    return indices;
}