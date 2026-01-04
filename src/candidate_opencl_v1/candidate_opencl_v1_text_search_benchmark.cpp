#include "candidate_opencl_v1_text_search_benchmark.h"
#include "candidate_opencl_v1_text_search.h"

#include <iostream>

Timer candidate_opencl_v1_timer = Timer(std::string("candidate_opencl_v1"));

std::vector<std::vector<int>>
benchmark_candidate_opencl_v1(const std::string &text,
                              const std::vector<std::string> &queries) {
    candidate_opencl_v1_timer.start_total();

    auto results = testCandidate_cl_v1(text, queries);

    candidate_opencl_v1_timer.stop_total();

    std::cout << "Finished candidate_opencl_v1 text search." << std::endl;

    candidate_opencl_v1_timer.print();

    return results;
}