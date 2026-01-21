#include "direct_opencl_text_search.h"

#include <CL/opencl.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>

#ifdef BENCHMARK
Timer direct_opencl_timer = Timer(std::string("direct_opencl"));
#endif

namespace {
const char *kernel_src = R"(
__kernel void test_candidates(
    __global const char* text,
    uint text_length,
    __global const char* flatten_queries,
    __global const uint* query_offsets,
    __global const uint* query_lengths,
    __global int* results,
    __global int* query_mappings,
    __global uint* counter,
    uint max_total_results)
{
    const size_t i = get_global_id(0);
    const size_t j = get_global_id(1);

    const uint query_offset = query_offsets[j];
    const uint query_length = query_lengths[j];

    if (i > text_length - query_length) {
        return;
    }

    const uint mid = query_length >> 1;
    const uint end = query_length - 1;

    bool candidate = text[i] == flatten_queries[query_offset] &&
                     text[i + mid] == flatten_queries[query_offset + mid] &&
                     text[i + end] == flatten_queries[query_offset + end];

    if (candidate) {
        bool match = true;

        for (int k = 0; k < query_length; k++) {
            if (k == mid) {
                continue;
            }

            if (text[i + k] != flatten_queries[query_offset + k]) {
                match = false;

                break;
            }
        }

        if (match) {
            int index = atomic_inc(counter);

            if (index < max_total_results) {
                results[index] = i;
                query_mappings[index] = j;
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
find_direct_opencl(const std::string &text,
                   const std::vector<std::string> &queries) {

    const auto query_amount = queries.size();
    const auto text_length = text.size();

    std::vector<std::vector<size_t>> indices(query_amount);

    constexpr uint32_t max_total_results = 20000000;

    cl_int err;
    cl_platform_id platform;
    cl_device_id device;

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

    const auto kernel = clCreateKernel(program, "test_candidates", &err);
    opencl(err, "clCreateKernel");

    std::vector<char> flatten_queries;
    std::vector<uint32_t> query_offsets(query_amount);
    std::vector<uint32_t> query_lengths(query_amount);

    for (size_t i = 0; i < query_amount; ++i) {
        query_offsets[i] = flatten_queries.size();
        query_lengths[i] = queries[i].length();
        flatten_queries.insert(flatten_queries.end(), queries[i].begin(),
                               queries[i].end());
    }

    const auto text_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       text_length, const_cast<char *>(text.data()), &err);
    opencl(err, "create text_buffer");

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

    const auto results_buffer =
        clCreateBuffer(context, CL_MEM_READ_WRITE,
                       max_total_results * sizeof(uint32_t), nullptr, &err);
    opencl(err, "clCreateBuffer results_buffer");

    const auto query_mappings_buffer =
        clCreateBuffer(context, CL_MEM_READ_WRITE,
                       max_total_results * sizeof(uint32_t), nullptr, &err);
    opencl(err, "clCreateBuffer query_mappings_buffer");

    uint32_t zero = 0;
    const auto counter_buf =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sizeof(uint32_t), &zero, &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &text_buffer);
    clSetKernelArg(kernel, 1, sizeof(uint32_t), &text_length);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &flatten_queries_buffer);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &query_offsets_buffer);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &query_lengths_buffer);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), &results_buffer);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), &query_mappings_buffer);
    clSetKernelArg(kernel, 7, sizeof(cl_mem), &counter_buf);
    clSetKernelArg(kernel, 8, sizeof(uint32_t), &max_total_results);

    constexpr size_t local_size[2] = {256, 1};
    const size_t global_size[2] = {(text_length + 255) / 256 * 256,
                                   query_amount};

    clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global_size, local_size,
                           0, nullptr, nullptr);

    uint32_t count = 0;

    clEnqueueReadBuffer(queue, counter_buf, CL_TRUE, 0, sizeof(uint32_t),
                        &count, 0, nullptr, nullptr);

    std::vector<uint32_t> results(count);
    std::vector<uint32_t> query_mappings(count);

    clEnqueueReadBuffer(queue, results_buffer, CL_TRUE, 0,
                        count * sizeof(uint32_t), results.data(), 0, nullptr,
                        nullptr);
    clEnqueueReadBuffer(queue, query_mappings_buffer, CL_TRUE, 0,
                        count * sizeof(uint32_t), query_mappings.data(), 0,
                        nullptr, nullptr);

    for (uint32_t i = 0; i < count; i++) {
        indices[query_mappings[i]].push_back(results[i]);
    }

    clReleaseMemObject(text_buffer);
    clReleaseMemObject(flatten_queries_buffer);
    clReleaseMemObject(query_offsets_buffer);
    clReleaseMemObject(query_lengths_buffer);
    clReleaseMemObject(results_buffer);
    clReleaseMemObject(query_mappings_buffer);
    clReleaseMemObject(counter_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return indices;
}