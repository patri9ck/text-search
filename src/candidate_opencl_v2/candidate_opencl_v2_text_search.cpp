#include "candidate_opencl_v2_text_search.h"

#include <CL/cl.h>
#include <cstring>
#include <sstream>

#ifdef BENCHMARK
Timer candidate_opencl_v2_timer = Timer(std::string("candidate_opencl_v2"));
#endif

namespace {
const char *kernel_src = R"(
__kernel void count_and_test_candidates(
    __global const char *text,
    __global uint* counts,
    __global uint* results,
    uint text_length,
    __constant const char *flatten_queries,
    __constant const uint *query_offsets,
    __constant const uint *query_lengths,
    __global const uint *result_offsets,
    uint mode
) {
    const size_t i = get_global_id(0);
    const size_t j = get_global_id(1);

    if (i >= text_length) {
        return;
    }

    const uint query_offset = query_offsets[j];
    const uint query_length = query_lengths[j];

    if (i + query_length > text_length) {
        return;
    }

    const uint mid = query_length >> 1;
    const uint end = query_length - 1;

    bool candidate = text[i] == flatten_queries[query_offset] &&
                     text[i + mid] == flatten_queries[query_offset + mid] &&
                     text[i + end] == flatten_queries[query_offset + end];

    if (candidate) {
        const uint index = j * get_num_groups(0) + get_group_id(0);

        if (mode == 1) {
            atomic_inc(&counts[index]);
        } else {
            bool match = true;

            for (uint k = 1; k < query_length - 1; ++k) {
                if (k == mid) {
                    continue;
                }

                if (text[i + k] != flatten_queries[query_offset + k]) {
                    match = false;

                    break;
                }
            }

            if (match) {
                uint count = atomic_inc(&counts[index]);

                results[result_offsets[index] + count] = (uint)i;
            }
        }
    }
}
)";

void opencl(const cl_int err, const char *function,
            const cl_program &program = nullptr,
            const cl_device_id &device = nullptr) {
    if (err != CL_SUCCESS) {
        std::stringstream ss;

        ss << "opencl error: " << function << std::endl;

        if (std::strcmp(function, "clBuildProgram") == 0 &&
            program != nullptr && device != nullptr) {
            size_t log_size;
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &log_size);
            std::vector<char> log(log_size);
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  log_size, log.data(), nullptr);
            ss << log.data();
        }

        const auto s = ss.str();

        throw std::runtime_error(s);
    }
}
} // namespace

std::vector<std::vector<size_t>>
find_candidate_opencl_v2(const std::string &text,
                         const std::vector<std::string> &queries) {
    const auto query_amount = queries.size();
    const auto text_length = text.length();

    std::vector<std::vector<size_t>> indices(query_amount);

    std::vector<char> flatten_queries;
    std::vector<uint32_t> query_offsets(query_amount);
    std::vector<uint32_t> query_lengths(query_amount);

    for (size_t i = 0; i < query_amount; ++i) {
        query_offsets[i] = flatten_queries.size();
        query_lengths[i] = queries[i].length();
        flatten_queries.insert(flatten_queries.end(), queries[i].begin(),
                               queries[i].end());
    }

    constexpr size_t local_size = 256;
    const size_t group_amount = (text_length + local_size - 1) / local_size;

    cl_platform_id platform;
    cl_int err;
    cl_device_id device;

    const auto total_amount = group_amount * query_amount;
    std::vector<uint32_t> counts(total_amount, 0);

    const size_t global_work_size[2] = {group_amount * local_size,
                                        query_amount};
    constexpr size_t local_work_size[2] = {local_size, 1};

    uint32_t mode = 1;

    opencl(clGetPlatformIDs(1, &platform, nullptr), "clGetPlatformIDs");
    opencl(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr),
           "glGetDeviceIDs");

    const auto context =
        clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    opencl(err, "clCreateContext");

    const auto queue =
        clCreateCommandQueueWithProperties(context, device, nullptr, &err);
    opencl(err, "clCreateCommandQueueWithProperties");

    const auto program =
        clCreateProgramWithSource(context, 1, &kernel_src, nullptr, &err);
    opencl(err, "clCreateProgramWithSource");

    opencl(clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr),
           "clBuildProgram", program, device);

    const auto kernel =
        clCreateKernel(program, "count_and_test_candidates", &err);
    opencl(err, "clCreateKernel");

    const auto text_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       text_length, const_cast<char *>(text.data()), &err);
    opencl(err, "clCreateBuffer text");

    const auto flatten_queries_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       flatten_queries.size(), flatten_queries.data(), &err);
    opencl(err, "clCreateBuffer flatten_queries");

    const auto query_offsets_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(uint32_t) * query_amount, query_offsets.data(), &err);
    opencl(err, "clCreateBuffer query_offsets");

    const auto query_lengths_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(uint32_t) * query_amount, query_lengths.data(), &err);
    opencl(err, "clCreateBuffer query_length");

    const auto counts_buffer =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       total_amount * sizeof(uint32_t), counts.data(), &err);
    opencl(err, "clCreateBuffer counts");

    opencl(clSetKernelArg(kernel, 0, sizeof(cl_mem), &text_buffer),
           "clSetKernelArg text");
    opencl(clSetKernelArg(kernel, 1, sizeof(cl_mem), &counts_buffer),
           "clSetKernelArg counts");
    opencl(clSetKernelArg(kernel, 2, sizeof(cl_mem), nullptr),
           "clSetKernelArg results");
    opencl(clSetKernelArg(kernel, 3, sizeof(uint32_t), &text_length),
           "clSetKernelArg text_length");
    opencl(clSetKernelArg(kernel, 4, sizeof(cl_mem), &flatten_queries_buffer),
           "clSetKernelArg flatten_queries");
    opencl(clSetKernelArg(kernel, 5, sizeof(cl_mem), &query_offsets_buffer),
           "clSetKernelArg query_offsets");
    opencl(clSetKernelArg(kernel, 6, sizeof(cl_mem), &query_lengths_buffer),
           "clSetKernelArg query_lengths");
    opencl(clSetKernelArg(kernel, 7, sizeof(cl_mem), nullptr),
           "clSetKernelArg result_offsets");
    opencl(clSetKernelArg(kernel, 8, sizeof(uint32_t), &mode),
           "clSetKernelArg mode");

    opencl(clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global_work_size,
                                  local_work_size, 0, nullptr, nullptr),
           "clEnqueueNDRangeKernel");
    opencl(clEnqueueReadBuffer(queue, counts_buffer, CL_TRUE, 0,
                               counts.size() * sizeof(uint32_t), counts.data(),
                               0, nullptr, nullptr),
           "clEnqueueReadBuffer");

    std::vector<uint32_t> result_offsets(total_amount, 0);

    uint32_t total_matches = 0;

    for (size_t i = 0; i < total_amount; ++i) {
        result_offsets[i] = total_matches;

        total_matches += counts[i];
    }

    const auto results_buffer = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY,
        std::max(1U, total_matches) * sizeof(uint32_t), nullptr, &err);
    opencl(err, "createBuffer results");

    const auto result_offsets_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        total_amount * sizeof(uint32_t), result_offsets.data(), &err);
    opencl(err, "createBuffer result_offsets");

    mode = 0;

    opencl(clSetKernelArg(kernel, 2, sizeof(cl_mem), &results_buffer),
           "clSetKernelArg results");
    opencl(clSetKernelArg(kernel, 7, sizeof(cl_mem), &result_offsets_buffer),
           "result_offsets_buffer");
    opencl(clSetKernelArg(kernel, 8, sizeof(uint32_t), &mode),
           "clSetKernelArg mode");

    constexpr uint32_t zero = 0;
    opencl(clEnqueueFillBuffer(queue, counts_buffer, &zero, sizeof(uint32_t), 0,
                               counts.size() * sizeof(uint32_t), 0, nullptr,
                               nullptr),
           "clEnqueueFillBuffer");
    opencl(clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global_work_size,
                                  local_work_size, 0, nullptr, nullptr),
           "clEnqueueNDRangeKernel");

    std::vector<uint32_t> results(total_matches);

    if (total_matches > 0) {
        opencl(clEnqueueReadBuffer(queue, results_buffer, CL_TRUE, 0,
                                   total_matches * sizeof(uint32_t),
                                   results.data(), 0, nullptr, nullptr),
               "clEnqueueReadBuffer results");

        opencl(clEnqueueReadBuffer(queue, counts_buffer, CL_TRUE, 0,
                                   counts.size() * sizeof(uint32_t),
                                   counts.data(), 0, nullptr, nullptr),
               "clEnqueueReadBuffer counts");
    }

    for (size_t j = 0; j < query_amount; ++j) {
        for (size_t k = 0; k < group_amount; ++k) {
            const auto flat_index = j * group_amount + k;

            const auto count = counts[flat_index];
            const auto offset = result_offsets[flat_index];

            for (uint32_t l = 0; l < count; ++l) {
                indices[j].push_back(results[offset + l]);
            }
        }
    }

    clReleaseMemObject(text_buffer);
    clReleaseMemObject(flatten_queries_buffer);
    clReleaseMemObject(query_offsets_buffer);
    clReleaseMemObject(query_lengths_buffer);
    clReleaseMemObject(result_offsets_buffer);
    clReleaseMemObject(counts_buffer);
    clReleaseMemObject(results_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return indices;
}