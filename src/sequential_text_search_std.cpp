#include "sequential_text_search_std.h"

std::vector<std::vector<int>> find_sequential_std(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices;

    for (int i = 0; i < queries.size(); ++i) {
        indices[i] = std::vector<int>();

        const std::string& query = queries[i];

        size_t pos = text.find(query);
        while (pos != std::string::npos) {
            indices[i].push_back(pos);

            pos = text.find(query, pos + 1);
        }
    }

    return indices;
}
