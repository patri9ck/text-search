#include "sequential_text_search_hash.h"

#include <string>
#include <vector>
#include <cstdint>

uint64_t hash_substring(const std::string &s, int start, int length) {
    const uint64_t prime = 31;
    uint64_t h = 0;
    for (int i = 0; i < length; ++i) {
        h = h * prime + s[start + i];
    }
    return h;
}

std::vector<std::vector<int>> find_sequential_hash(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices;
    int text_length = text.length();

    for (const auto& query : queries) {
        std::vector<int> query_indices;
        int query_length = query.length();
        uint64_t query_hash = hash_substring(query, 0, query_length);

        for (int i = 0; i <= text_length - query_length; ++i) {
            if (hash_substring(text, i, query_length) == query_hash) {
                // Optional: nochmal Zeichenvergleich, um Hash-Kollisionen auszuschließen
                bool match = true;
                for (int j = 0; j < query_length; ++j) {
                    if (text[i + j] != query[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) query_indices.push_back(i);
            }
        }

        indices.push_back(query_indices);
    }

    return indices;
}
