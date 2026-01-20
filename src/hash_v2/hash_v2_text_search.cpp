#include "hash_v2_text_search.h"

#include <unordered_map>
#include <algorithm>

#define PRIME 1000003

#ifdef BENCHMARK
Timer hash_v2_timer = Timer(std::string("hash_v1"));
#endif

namespace {

    uint64_t compute_power(size_t length) {
        uint64_t p = 1;
        for (size_t i = 1; i < length; ++i) {
            p *= PRIME;
        }
        return p;
    }

}

std::vector<std::vector<size_t>>
hash_v2(const std::string &text, const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());
    const size_t text_length = text.length();

    // 1. Gruppierung nach Länge: map<Länge, Vektor<Query-Index>>
    std::unordered_map<size_t, std::vector<size_t>> length_groups;
    for (size_t i = 0; i < queries.size(); ++i) {
        length_groups[queries[i].length()].push_back(i);
    }

    // 2. Pro Längengruppe den Text GENAU EINMAL scannen
    for (auto const& [len, q_indices] : length_groups) {
        if (len > text_length || len == 0) continue;

        // Hashes für alle Queries dieser Länge vorbereiten
        // map<Hash, Vektor<Query-Index>> für schnellen Lookup
        std::unordered_map<uint64_t, std::vector<size_t>> hash_table;
        for (size_t q_idx : q_indices) {
            uint64_t h = 0;
            for (char c : queries[q_idx]) h = h * PRIME + (uint8_t)c;
            hash_table[h].push_back(q_idx);
        }

        uint64_t power = compute_power(len);
        uint64_t window_hash = 0;

        // Ersten Window-Hash berechnen
        for (size_t i = 0; i < len; ++i) window_hash = window_hash * PRIME + (uint8_t)text[i];

        // Einmaliger Scan durch den Text für ALLE Queries dieser Länge
        for (size_t j = 0; j <= text_length - len; ++j) {

            // Look-up in der Hash-Tabelle: O(1) im Durchschnitt!
            auto it = hash_table.find(window_hash);
            if (it != hash_table.end()) {
                // Nur bei Hash-Treffer: Validierung für alle betroffenen Queries
                for (size_t q_idx : it->second) {
                    if (memcmp(&text[j], queries[q_idx].data(), len) == 0) {
                        indices[q_idx].push_back(j);
                    }
                }
            }

            // Rolling Hash: Nächster Schritt
            if (j < text_length - len) {
                window_hash -= static_cast<uint64_t>((uint8_t)text[j]) * power;
                window_hash *= PRIME;
                window_hash += (uint8_t)text[j + len];
            }
        }
    }
    return indices;
}