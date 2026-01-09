#include "naive_v1_text_search.h"

#ifdef BENCHMARK
Timer naive_v1_timer = Timer(std::string("naive_v1"));
#endif

std::vector<std::vector<size_t>>
naive_v1(const std::string &text, const std::vector<std::string> &queries) {

    std::vector<std::vector<size_t>> indices(queries.size());
    const size_t text_length = text.length();

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];
        const size_t m = query.length();

        if (m == 0 || m > text_length) continue;

        for (size_t j = 0; j <= text_length - m; ++j) {
            bool match = true;

            for (size_t k = 0; k < m; ++k) {
                if (text[j + k] != query[k]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                indices[i].push_back(j);
            }
        }
    }

    return indices;
}