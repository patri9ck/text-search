#include "candidate_opencl_v2_text_search.h"

#include <CL/cl.h>
#include <cstring>

#ifdef BENCHMARK
Timer candidate_opencl_v2_timer = Timer(std::string("candidate_opencl_v2"));
#endif

namespace {
const char *kernel_src = R"(
__kernel void find_and_test_candidates(
    __constant ulong *mask,
    ulong mask_words,
    __global const char *text,
    ulong text_length,
    __constant const char *flatten_queries,
    __constant const uint *query_offsets,
    __constant const uint *query_lengths,
    uint num_queries
) {
    const size_t i = get_global_id(0);
    const size_t j = get_global_id(1);

    const uint query_offset = query_offsets[j];
    const uint query_length = query_lengths[j];

    const uint mid = query_length >> 1;
    const uint end = query_length - 1;

    if (i + end >= text_length) {
        return;
    }

    if (text[i] == flatten_queries[query_offset] &&
        text[i + mid] == flatten_queries[query_offset + mid] &&
        text[i + end] == flatten_queries[query_offset + end]) {

        atom_or(&mask[j * mask_words + (i / 64)], (ulong)1 << (i % 64));
    }
}

__kernel void test_candidates(
    __global const ulong *mask,
    ulong mask_words,
    __global const char *text,
    ulong text_length,
    __global const char *flatten_queries,
    __global const uint *query_offsets,
    __global const uint *query_lengths,
    uint num_queries,
    __global uint *results_size,
    __global ulong *results,
) {
    const size_t i = get_global_id(0);
    const size_t j = get_global_id(1);

    const uint query_offset = query_offsets[j];
    const uint query_length = query_lengths[j];

    ulong w = mask[j * mask_words + i];

    while (w != 0) {
        ulong index = i * 64 + ctz(w);

        for (uint k = 0; k < query_length; ++k) {
            if (text[index + k] != flatten_queries[query_offset + k]) {
                w &= (w - 1);

                return;
            }
        }

        match_indices[query_id * max_matches_per_query + atomic_inc(&match_counts[j])] = index;

        w &= (w - 1);
    }
}
)";
} // namespace

std::vector<std::vector<size_t>>
find_candidate_opencl_v2(const std::string &text,
                         const std::vector<std::string> &queries) {
    std::vector<std::vector<size_t>> indices(queries.size());

    unsigned long mask_words = (text.length() + 63) / 64;
    auto *mask = new uint64_t[mask_words * queries.size()]();

    auto query_amount = queries.size();

    std::vector<char> flatten_queries;

    auto query_offsets = new uint32_t[query_amount];
    auto query_lengths = new uint32_t[query_amount];

    for (size_t i = 0; i < query_amount; ++i) {
        const auto &query = queries[i];

        query_offsets[i] = flatten_queries.size();
        query_lengths[i] = query.length();

        flatten_queries.insert(flatten_queries.end(), query.begin(),
                               query.end());
    }

    const auto text_length = text.length();

    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, nullptr);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    const auto context =
        clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);

    const auto queue =
        clCreateCommandQueueWithProperties(context, device, nullptr, nullptr);

    const auto program =
        clCreateProgramWithSource(context, 1, &kernel_src, nullptr, nullptr);

    clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);

    const auto find_candidates_kernel =
        clCreateKernel(program, "find_candidates", nullptr);

    const auto mask_buffer = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        mask_words * queries.size() * sizeof(uint64_t), mask, nullptr);

    const auto text_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, text.size(),
        const_cast<void *>(static_cast<const void *>(text.data())), nullptr);

    cl_mem queries_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       flatten_queries.size(), flatten_queries.data(), nullptr);

    cl_mem offsets_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       query_amount * sizeof(uint32_t), query_offsets, nullptr);

    cl_mem lengths_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       query_amount * sizeof(uint32_t), query_lengths, nullptr);

    clSetKernelArg(find_candidates_kernel, 0, sizeof(cl_mem), &mask_buffer);
    clSetKernelArg(find_candidates_kernel, 1, sizeof(unsigned long),
                   &mask_words);
    clSetKernelArg(find_candidates_kernel, 2, sizeof(cl_mem), &text_buffer);
    clSetKernelArg(find_candidates_kernel, 3, sizeof(size_t), &text_length);
    clSetKernelArg(find_candidates_kernel, 4, sizeof(cl_mem), &queries_buffer);
    clSetKernelArg(find_candidates_kernel, 5, sizeof(cl_mem), &offsets_buffer);
    clSetKernelArg(find_candidates_kernel, 6, sizeof(cl_mem), &lengths_buffer);
    clSetKernelArg(find_candidates_kernel, 7, sizeof(uint32_t), &query_amount);

    size_t global_work_size[2] = {text_length, query_amount};

    clEnqueueNDRangeKernel(queue, find_candidates_kernel, 2, nullptr,
                           global_work_size, nullptr, 0, nullptr, nullptr);

    clEnqueueReadBuffer(queue, mask_buffer, CL_TRUE, 0,
                        mask_words * queries.size() * sizeof(uint64_t), mask, 0,
                        nullptr, nullptr);

    auto *results_size = new uint32_t[query_amount]();
    auto *results = new uint64_t[text_length]();

    cl_mem results_size_buffer =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                       query_amount * sizeof(uint32_t), results_size, nullptr);

    cl_mem results_buffer =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                       query_amount * sizeof(uint64_t), results, nullptr);

    const auto test_candidates_kernel =
        clCreateKernel(program, "test_candidates", nullptr);

    clSetKernelArg(test_candidates_kernel, 0, sizeof(cl_mem), &mask_buffer);
    clSetKernelArg(test_candidates_kernel, 1, sizeof(unsigned long),
                   &mask_words);
    clSetKernelArg(test_candidates_kernel, 2, sizeof(cl_mem), &text_buffer);
    clSetKernelArg(test_candidates_kernel, 3, sizeof(size_t), &text_length);
    clSetKernelArg(test_candidates_kernel, 4, sizeof(cl_mem), &queries_buffer);
    clSetKernelArg(test_candidates_kernel, 5, sizeof(cl_mem), &offsets_buffer);
    clSetKernelArg(test_candidates_kernel, 6, sizeof(cl_mem), &lengths_buffer);
    clSetKernelArg(test_candidates_kernel, 7, sizeof(uint32_t), &query_amount);
    clSetKernelArg(test_candidates_kernel, 8, sizeof(cl_mem),
                   &results_size_buffer);
    clSetKernelArg(test_candidates_kernel, 9, sizeof(cl_mem), &results_buffer);

    global_work_size[0] = mask_words;
    global_work_size[1] = query_amount;

    clEnqueueNDRangeKernel(queue, test_candidates_kernel, 2, nullptr,
                           global_work_size, nullptr, 0, nullptr, nullptr);

    clEnqueueReadBuffer(queue, mask_buffer, CL_TRUE, 0,
                        mask_words * queries.size() * sizeof(uint64_t), mask, 0,
                        nullptr, nullptr);

    for (size_t i = 0; i < query_amount; ++i) {
        uint32_t count = results_size[i];

        for (uint32_t j = 0; j < count; ++j) {
            uint64_t index = results[i * text_length + j];
            indices[i].push_back(index);
        }
    }

    clReleaseMemObject(mask_buffer);
    clReleaseMemObject(text_buffer);
    clReleaseMemObject(queries_buffer);
    clReleaseMemObject(offsets_buffer);
    clReleaseMemObject(lengths_buffer);
    clReleaseMemObject(results_size_buffer);
    clReleaseMemObject(results_buffer);
    clReleaseKernel(find_candidates_kernel);
    clReleaseKernel(test_candidates_kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    delete[] mask;
    delete[] query_offsets;
    delete[] query_lengths;
    delete[] results_size;
    delete[] results;

    return indices;
}