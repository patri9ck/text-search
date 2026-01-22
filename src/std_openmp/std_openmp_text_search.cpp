#include "std_openmp_text_search.h"

#include <omp.h>

#ifdef BENCHMARK
Timer std_openmp_timer = Timer(std::string("std_openmp"));
#endif

std::vector<std::vector<size_t>>
find_std_openmp(const std::string &text,
                const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    const int max_threads = omp_get_max_threads();

    omp_set_num_threads(max_threads);

#pragma omp parallel for default(none) shared(queries, text, indices)
    for (long i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        auto pos = text.find(query);

        while (pos != std::string::npos) {
            indices[i].push_back(pos);

            pos = text.find(query, pos + 1);
        }
    }

    return indices;
}
