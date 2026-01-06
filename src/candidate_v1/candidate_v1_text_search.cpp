#include "candidate_v1_text_search.h"

#include <cstring>

#ifdef BENCHMARK
Timer candidate_v1_timer = Timer(std::string("candidate_v1"));
#endif

namespace {
void find_candidates(const size_t text_length, std::vector<size_t> &results,
                     const std::string &text, const std::string &query) {
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    for (size_t i = 0; i < text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
            results.push_back(i);
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

    const auto text_length = text.length();

    std::vector<size_t> results;

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        find_candidates(text_length, results, text, query);

        for (const auto &result : results) {
            if (test_candidate(result, text, query)) {
                indices[i].push_back(result);
            }
        }

        std::memset(results.data(), 0, results.size() * sizeof(size_t));
    }

    return indices;
}