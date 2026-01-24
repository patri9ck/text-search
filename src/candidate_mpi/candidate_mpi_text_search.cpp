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

void find_candidates(const size_t text_length, uint64_t *mask,
                     const std::string_view &text, const std::string &query) {
    const auto query_length = query.length();
    const auto mid = query_length >> 1;
    const auto end = query_length - 1;

    for (size_t i = 0; i + query_length <= text_length; ++i) {
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

    auto copied_text = text;
    auto copied_queries = queries;

    int text_length = static_cast<int>(copied_text.length());
    int query_amount = static_cast<int>(copied_queries.size());

    MPI_Bcast(&text_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&query_amount, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        copied_queries.resize(query_amount);
    }

    int maximum_query_length = 0;

    for (int i = 0; i < query_amount; ++i) {
        int query_length =
            rank == 0 ? static_cast<int>(copied_queries[i].size()) : 0;

        MPI_Bcast(&query_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0) {
            copied_queries[i].resize(query_length);
        }

        MPI_Bcast(copied_queries[i].data(), query_length, MPI_CHAR, 0,
                  MPI_COMM_WORLD);

        maximum_query_length = std::max(maximum_query_length, query_length);
    }

    const int overlap = maximum_query_length > 0 ? maximum_query_length - 1 : 0;
    const int base_chunk_size = text_length / size;

    std::vector<int> send_counts(size);
    std::vector<int> displacements(size);

    for (int r = 0; r < size; ++r) {
        displacements[r] = r * base_chunk_size;
        send_counts[r] =
            (r == size - 1 ? text_length - displacements[r] : base_chunk_size) +
            (r == size - 1 ? 0 : overlap);
    }

    const int local_length =
        rank == size - 1 ? text_length - displacements[rank] : base_chunk_size;

    std::string local_buffer(send_counts[rank], '\0');

    MPI_Scatterv(copied_text.data(), send_counts.data(), displacements.data(),
                 MPI_CHAR, local_buffer.data(), send_counts[rank], MPI_CHAR, 0,
                 MPI_COMM_WORLD);

    const std::string_view local_text(local_buffer.data(), local_buffer.size());

    std::vector<std::vector<size_t>> local_indices(query_amount);

    const auto local_text_length = local_text.length();

    const size_t mask_words = (local_text_length + 63) / 64;
    std::vector<uint64_t> mask(mask_words);

    for (int i = 0; i < query_amount; ++i) {
        const auto &query = copied_queries[i];

        if (query.length() > local_text_length) {
            continue;
        }

        find_candidates(local_text_length, mask.data(), local_text, query);

        for (size_t word = 0; word < mask_words; ++word) {
            uint64_t w = mask[word];

            while (w != 0) {
                size_t index = word * 64 + countr_zero(w);

                if (index + query.length() <= local_text_length &&
                    index < static_cast<size_t>(local_length) &&
                    test_candidate(index, local_text, query)) {
                    local_indices[i].push_back(displacements[rank] + index);
                }

                w &= w - 1;
            }
        }

        std::fill(mask.begin(), mask.end(), 0);
    }

    std::vector<std::vector<size_t>> indices;

    if (rank == 0) {
        indices.resize(query_amount);
    }

    for (int i = 0; i < query_amount; ++i) {
        int local_count = static_cast<int>(local_indices[i].size());

        std::vector<int> counts(size);

        MPI_Gather(&local_count, 1, MPI_INT, counts.data(), 1, MPI_INT, 0,
                   MPI_COMM_WORLD);

        if (rank == 0) {
            std::vector<int> offsets(size);
            int total = 0;

            for (int r = 0; r < size; ++r) {
                offsets[r] = total;
                total += counts[r];
            }

            indices[i].resize(total);

            MPI_Gatherv(local_indices[i].data(), local_count, MPI_UINT64_T,
                        indices[i].data(), counts.data(), offsets.data(),
                        MPI_UINT64_T, 0, MPI_COMM_WORLD);
        } else {
            MPI_Gatherv(local_indices[i].data(), local_count, MPI_UINT64_T,
                        nullptr, nullptr, nullptr, MPI_UINT64_T, 0,
                        MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return indices;
}
