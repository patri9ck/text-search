#include "std_text_search.h"

#ifdef BENCHMARK
Timer std_timer = Timer(std::string("std"));
#endif

std::vector<std::vector<size_t>>
find_std(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        auto pos = text.find(query);

        while (pos != std::string::npos) {
            indices[i].push_back(pos);

            pos = text.find(query, pos + 1);
        }
    }

    return indices;
}
