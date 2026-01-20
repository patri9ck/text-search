#include "hash_openmp_v1_text_search.h"
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <omp.h>

#define PRIME 1000003

#ifdef BENCHMARK
Timer hash_openmp_v1_timer = Timer(std::string("hash_openmp_v1"));
#endif

namespace {
    uint64_t compute_power(size_t length) {
        uint64_t p = 1;
        for (size_t i = 1; i < length; ++i) {
            p *= PRIME;
        }
        return p;
    }

    struct LengthBlock {
        size_t length;
        size_t start_idx; // Start im sortierten q_indices Vektor
        size_t count;     // Anzahl der Queries mit dieser Länge
    };
}

std::vector<std::vector<size_t>>
find_hash_openmp_v1(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());
    const size_t text_length = text.length();
    const size_t num_queries = queries.size();

    if (num_queries == 0 || text_length == 0) return indices;

    // 1. Indizes sortieren nach Query-Länge (statt Map)
    std::vector<size_t> q_indices(num_queries);
    std::iota(q_indices.begin(), q_indices.end(), 0);

    std::sort(q_indices.begin(), q_indices.end(), [&](size_t a, size_t b) {
        return queries[a].length() < queries[b].length();
    });

    // 2. Blöcke gleicher Länge identifizieren
    std::vector<LengthBlock> blocks;
    for (size_t i = 0; i < num_queries; ) {
        size_t len = queries[q_indices[i]].length();
        size_t start = i;
        while (i < num_queries && queries[q_indices[i]].length() == len) {
            i++;
        }
        if (len > 0 && len <= text_length) {
            blocks.push_back({len, start, i - start});
        }
    }

    // 3. Parallel über die Längen-Blöcke iterieren
#pragma omp parallel for schedule(dynamic)
    for (long b = 0; b < blocks.size(); ++b) {
        const size_t current_len = blocks[b].length;
        const size_t start_in_sorted = blocks[b].start_idx;
        const size_t block_size = blocks[b].count;

        // Lokale Hash-Tabelle für diesen Thread/Länge
        // Key: Hash, Value: Index der Query
        std::unordered_map<uint64_t, std::vector<size_t>> hash_to_queries;
        for (size_t i = 0; i < block_size; ++i) {
            size_t q_idx = q_indices[start_in_sorted + i];
            uint64_t h = 0;
            for (char c : queries[q_idx]) h = h * PRIME + (uint8_t)c;
            hash_to_queries[h].push_back(q_idx);
        }

        uint64_t power = compute_power(current_len);
        uint64_t window_hash = 0;

        // Ersten Window-Hash berechnen
        for (size_t i = 0; i < current_len; ++i) {
            window_hash = window_hash * PRIME + (uint8_t)text[i];
        }

        // Suche im Text
        for (size_t j = 0; j <= text_length - current_len; ++j) {
            auto it = hash_to_queries.find(window_hash);
            if (it != hash_to_queries.end()) {

                for (size_t q_idx : it->second) {
                    if (memcmp(&text[j], queries[q_idx].data(), current_len) == 0) {
                        indices[q_idx].push_back(j);
                    }
                }
            }

            // Roll den Hash weiter (außer am Ende)
            if (j < text_length - current_len) {
                window_hash -= static_cast<uint64_t>((uint8_t)text[j]) * power;
                window_hash *= PRIME;
                window_hash += (uint8_t)text[j + current_len];
            }
        }
    }

    return indices;
}