#include "candidate_opencl_v2_text_search.h"

#include <CL/cl.h>
#include <cstdint>
#include <cstring>

#ifdef BENCHMARK
Timer candidate_opencl_v2_timer = Timer(std::string("candidate_opencl_v2"));
#endif

namespace {
const char *find_candidates_kernel = R"(
__kernel void find_candidates(
    __global ulong *mask,
    ulong mask_words,
    __global const char *text,
    ulong text_length,
    __global const char *queries,
    __global const uint *query_offsets,
    __global const uint *query_lengths,
    uint num_queries
) {
    const size_t i = get_global_id(0);
    const size_t j = get_global_id(1);

    if (i >= text_length || j >= num_queries) {
        return;
    }

    const uint query_offset = query_offsets[j];
    const uint query_length = query_lengths[j];

    // Check if we have enough text left
    if (text_length - i < query_length) {
        return;
    }

    const uint mid = query_length >> 1;
    const uint end = query_length - 1;

    // Three-point check
    if (text[i] == queries[query_offset] &&
        text[i + mid] == queries[query_offset + mid] &&
        text[i + end] == queries[query_offset + end]) {

        // Set the bit in the mask
        const ulong word_idx = j * mask_words + (i >> 6);
        const ulong bit_pos = i & 63;

        atom_or(&mask[word_idx], (ulong)1 << bit_pos);
    }
}
)";

void find_candidates(uint64_t *mask, unsigned long mask_words,
                     const std::string &text,
                     const std::vector<std::string> &queries,
                     cl_context context, cl_command_queue queue,
                     cl_kernel kernel) {

    std::vector<char> queries_flat;
    std::vector<uint32_t> offsets, lengths;

    for (const auto &q : queries) {
        offsets.push_back(queries_flat.size());
        lengths.push_back(q.size());
        queries_flat.insert(queries_flat.end(), q.begin(), q.end());
    }

    cl_mem mask_buffer = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        mask_words * queries.size() * sizeof(uint64_t), mask, nullptr);

    cl_mem text_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       text.size(), (void *)text.data(), nullptr);

    cl_mem queries_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       queries_flat.size(), queries_flat.data(), nullptr);

    cl_mem offsets_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        offsets.size() * sizeof(uint32_t), offsets.data(), nullptr);

    cl_mem lengths_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        lengths.size() * sizeof(uint32_t), lengths.data(), nullptr);

    // 3. Set kernel arguments
    size_t text_length = text.size();
    uint32_t num_queries = queries.size();

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &mask_buffer);
    clSetKernelArg(kernel, 1, sizeof(unsigned long), &mask_words);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &text_buffer);
    clSetKernelArg(kernel, 3, sizeof(size_t), &text_length);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &queries_buffer);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), &offsets_buffer);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), &lengths_buffer);
    clSetKernelArg(kernel, 7, sizeof(uint32_t), &num_queries);

    const size_t global_work_size[2] = {text_length, num_queries};

    clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global_work_size, nullptr,
                           0, nullptr, nullptr);
}

bool test_candidate(const size_t index, const std::string &text,
                    const std::string &query) {
    return std::memcmp(text.data() + index, query.data(), query.size()) == 0;
}
} // namespace

std::vector<std::vector<size_t>>
find_candidate_opencl_v2(const std::string &text,
                         const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    unsigned long mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, nullptr);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    const auto context =
        clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);

    cl_command_queue queue =
        clCreateCommandQueueWithProperties(context, device, nullptr, nullptr);

    const auto program = clCreateProgramWithSource(
        context, 1, &find_candidates_kernel, nullptr, nullptr);

    clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);

    const auto kernel = clCreateKernel(program, "find_candidates", nullptr);

    find_candidates(mask, mask_words, text, queries, context, queue, kernel);

    for (size_t i = 0; i < queries.size(); ++i) {
        const std::string &query = queries[i];

        for (unsigned long word = 0; word < mask_words; ++word) {
            auto w = mask[i * mask_words + word];

            if (w == 0) {
                continue;
            }

            while (w != 0) {
                auto index = word * 64 + std::countr_zero(w);

                if (test_candidate(index, text, query)) {
                    indices[i].push_back(index);
                }

                w &= (w - 1);
            }
        }
    }

#ifdef BENCHMARK
    candidate_opencl_v2_timer.stop_sequential_part(2);
#endif

    delete[] mask;

    return indices;
}