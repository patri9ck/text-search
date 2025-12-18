#include "hash_text_search.h"

#include <cstdint>
#include <string>
#include <vector>

uint64_t compute_power(uint64_t prime, int length) {
    uint64_t p = 1;
    for (int i = 1; i < length; ++i) {
        p *= prime;
    }
    return p;
}

std::vector<std::vector<int>>
find_hash(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<int>> indices;
    int text_length = text.length();

    const uint64_t prime = 131;

    for (const auto &query : queries) {
        std::vector<int> query_indices;

        indices.push_back(query_indices);

        int m = query.length();

        uint64_t query_hash = 0;

        for (int i = 0; i < m; ++i) {
            query_hash = query_hash * prime + query[i];
        }

        uint64_t power = compute_power(prime, m);

        uint64_t window_hash = 0;
        for (int i = 0; i < m; ++i) {
            window_hash = window_hash * prime + text[i];
        }

        bool match = true;

        if (window_hash == query_hash) {
            for (int i = 0; i < m; ++i) {
                if (text[i] != query[i]) {
                    match = false;

                    break;
                }
            }

            if (match) {
                query_indices.push_back(0);
            }
        }

        for (int i = 1; i <= text_length - m; ++i) {
            match = true;

            window_hash -= static_cast<uint64_t>(text[i - 1]) * power;
            window_hash *= prime;
            window_hash += text[i + m - 1];

            if (window_hash == query_hash) {
                for (int j = 0; j < m; ++j) {
                    if (text[j + i] != query[j]) {
                        match = false;

                        break;
                    }
                }

                if (match) {
                    query_indices.push_back(i);
                }
            }
        }

        indices.push_back(query_indices);
    }

    return indices;
}
