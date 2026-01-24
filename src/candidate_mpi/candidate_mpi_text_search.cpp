#include "candidate_mpi_text_search.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <mpi.h>

#include "../util.h"

#ifdef BENCHMARK
Timer candidate_mpi_timer = Timer(std::string("candidate_mpi"));
#endif

namespace {

size_t get_max_query_length(const std::vector<std::string> &queries) {
    size_t max_len = 0;
    for (const auto &q : queries)
        max_len = std::max(max_len, q.length());
    return max_len;
}

void find_candidates(const size_t text_length, uint64_t *mask,
                     const std::string_view &text, const std::string &query) {
    const auto query_length = query.length();

    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    for (size_t i = 0; i <= text_length - query_length; ++i) {
        if (text[i] == query[0] && text[i + mid] == query[mid] &&
            text[i + end] == query[end]) {
            mask[i >> 6] |= static_cast<uint64_t>(1) << (i & 63);
        }
    }
}

bool test_candidate(const size_t index, const std::string_view text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}

} // namespace

std::vector<std::vector<size_t>>
find_candidate_mpi(const std::string &text,
                   const std::vector<std::string> &queries) {

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    size_t total_length = text.length();
    size_t max_q_len = get_max_query_length(queries);
    size_t overlap = (max_q_len > 0) ? max_q_len - 1 : 0;

    size_t base_chunk_size = total_length / size;
    size_t start_offset = rank * base_chunk_size;
    size_t local_length =
        (rank == size - 1) ? (total_length - start_offset) : base_chunk_size;

    size_t send_length = local_length + ((rank == size - 1) ? 0 : overlap);
    std::string_view local_text(text.data() + start_offset, send_length);

    std::vector<std::vector<size_t>> local_indices(queries.size());

    // Maske jetzt für LOKALEN Text
    const size_t local_text_length = local_text.length();
    const unsigned long mask_words = (local_text_length + 63) / 64;
    auto *mask = new uint64_t[mask_words]();

    for (size_t qi = 0; qi < queries.size(); ++qi) {
        const auto &query = queries[qi];
        if (query.empty() || query.length() > local_text_length)
            continue;

        find_candidates(local_text_length, mask, local_text, query);

        for (unsigned long word = 0; word < mask_words; ++word) {
            uint64_t w = mask[word];

            while (w != 0) {
                size_t bit = countr_zero(w);
                size_t local_index = word * 64 + bit;

                // Nur prüfen wenn Query komplett im lokalen Text liegt
                if (local_index + query.length() <= local_text_length) {

                    if (test_candidate(local_index, local_text, query)) {

                        // Nur Matches speichern, die im echten Chunk starten
                        if (local_index < local_length) {
                            size_t global_index = start_offset + local_index;
                            local_indices[qi].push_back(global_index);
                        }
                    }
                }

                w &= w - 1;
            }
        }

        std::memset(mask, 0, mask_words * sizeof(uint64_t));
    }

    delete[] mask;

    // einsammeln von den ranks in rank[0}
    std::vector<std::vector<size_t>> global_indices;
    if (rank == 0)
        global_indices.resize(queries.size());

    for (size_t qi = 0; qi < queries.size(); ++qi) {

        int local_count = static_cast<int>(local_indices[qi].size());
        std::vector<int> recv_counts(size);

        MPI_Gather(&local_count, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, 0,
                   MPI_COMM_WORLD);

        std::vector<int> displs;
        size_t total_recv = 0;

        if (rank == 0) {
            displs.resize(size);
            for (int r = 0; r < size; ++r) {
                displs[r] = total_recv;
                total_recv += recv_counts[r];
            }
            global_indices[qi].resize(total_recv);
        }

        MPI_Gatherv(local_indices[qi].data(), local_count, MPI_UNSIGNED_LONG,
                    rank == 0 ? global_indices[qi].data() : nullptr,
                    rank == 0 ? recv_counts.data() : nullptr,
                    rank == 0 ? displs.data() : nullptr, MPI_UNSIGNED_LONG, 0,
                    MPI_COMM_WORLD);
    }
    return global_indices;
}
