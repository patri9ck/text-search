#include "candidate_opencl_v1_text_search.h"

#include <CL/cl.h>
#include <sstream>

#include "../util.h"

#include <cstring>

#ifdef BENCHMARK
Timer candidate_opencl_v1_timer = Timer(std::string("candidate_opencl_v1"));
#endif

namespace {
const char *kernel_src = R"(
__kernel void test_candidates(
    __global uint *mask,
    uint group_amount,
    __global const char *text,
    uint text_length,
    __constant const char *flatten_queries,
    __constant const uint *query_offsets,
    __constant const uint *query_lengths
) {
    const uint i = get_global_id(0);
    const uint j = get_global_id(1);

    if (i >= text_length) {
        return;
    }

    const uint query_offset = query_offsets[j];
    const uint query_length = query_lengths[j];

    if (i + query_length > text_length) {
        return;
    }

    const uint k = get_local_id(0);
    const uint l = get_group_id(0);

    const uint mid = query_length >> 1;
    const uint end = query_length - 1;

    bool candidate = text[i] == flatten_queries[query_offset] &&
                     text[i + mid] == flatten_queries[query_offset + mid] &&
                     text[i + end] == flatten_queries[query_offset + end];

    if (candidate) {
        bool match = true;

        for (uint m = 1; m < query_length - 1; ++m) {
            if (m == mid) {
                continue;
            }

            if (text[i + m] != flatten_queries[query_offset + m]) {
                match = false;

                break;
            }
        }

        if (match) {
            atomic_or(&mask[(j * group_amount * 8) + (l * 8) + (k >> 5)], (uint)1 << (k & 31));
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
find_candidate_opencl_v1(const std::string &text,
                         const std::vector<std::string> &queries) {
    const auto query_amount = queries.size();
    const auto text_length = text.length();

    std::vector<std::vector<size_t>> indices(query_amount);

    constexpr size_t local_size = 256;
    const uint32_t group_amount = (text_length + local_size - 1) / local_size;

    const size_t mask_words = query_amount * group_amount * 8;

    std::vector<uint32_t> mask(mask_words, 0);

    std::vector<char> flatten_queries;
    std::vector<uint32_t> query_offsets(query_amount);
    std::vector<uint32_t> query_lengths(query_amount);

    for (size_t i = 0; i < query_amount; ++i) {
        query_offsets[i] = flatten_queries.size();
        query_lengths[i] = queries[i].length();
        flatten_queries.insert(flatten_queries.end(), queries[i].begin(),
                               queries[i].end());
    }

    cl_platform_id platform;
    cl_device_id device;
    cl_int err;

    opencl(clGetPlatformIDs(1, &platform, nullptr), "clGetPlatformIDs");
    opencl(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr),
           "clGetDeviceIDs");

    const auto context =
        clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    opencl(err, "clCreateContext");

    const auto queue =
        clCreateCommandQueueWithProperties(context, device, nullptr, &err);
    opencl(err, "clCreateCommandQueue");

    const auto program =
        clCreateProgramWithSource(context, 1, &kernel_src, nullptr, &err);
    opencl(err, "clCreateProgramWithSource");

    opencl(clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr),
           "clBuildProgram", program, device);

    const auto kernel = clCreateKernel(program, "test_candidates", &err);
    opencl(err, "clCreateKernel");

    const auto mask_buffer =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       mask_words * sizeof(uint32_t), mask.data(), &err);
    opencl(err, "create mask_buffer");

    const auto text_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       text_length, const_cast<char *>(text.data()), &err);
    opencl(err, "create text_buffer");

    const auto queries_buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       flatten_queries.size(), flatten_queries.data(), &err);
    opencl(err, "create queries_buffer");

    const auto offsets_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        query_amount * sizeof(uint32_t), query_offsets.data(), &err);
    opencl(err, "create offsets_buffer");

    const auto lengths_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        query_amount * sizeof(uint32_t), query_lengths.data(), &err);
    opencl(err, "create lengths_buffer");

    opencl(clSetKernelArg(kernel, 0, sizeof(cl_mem), &mask_buffer),
           "clSetKernelArg mask");
    opencl(clSetKernelArg(kernel, 1, sizeof(uint32_t), &group_amount),
           "clSetKernelArg group_amount");
    opencl(clSetKernelArg(kernel, 2, sizeof(cl_mem), &text_buffer),
           "clSetKernelArg text");
    opencl(clSetKernelArg(kernel, 3, sizeof(uint32_t), &text_length),
           "clSetKernelArg text_length");
    opencl(clSetKernelArg(kernel, 4, sizeof(cl_mem), &queries_buffer),
           "clSetKernelArg flatten_queries");
    opencl(clSetKernelArg(kernel, 5, sizeof(cl_mem), &offsets_buffer),
           "clSetKernelArg query_offsets");
    opencl(clSetKernelArg(kernel, 6, sizeof(cl_mem), &lengths_buffer),
           "clSetKernelArg query_lengths");

    const size_t global_work_size[2] = {group_amount * local_size,
                                        query_amount};
    constexpr size_t local_work_size[2] = {local_size, 1};

    opencl(clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global_work_size,
                                  local_work_size, 0, nullptr, nullptr),
           "clEnqueueNDRangeKernel");
    opencl(clEnqueueReadBuffer(queue, mask_buffer, CL_TRUE, 0,
                               mask.size() * sizeof(uint32_t), mask.data(), 0,
                               nullptr, nullptr),
           "clReadBuffer");

    for (size_t j = 0; j < query_amount; ++j) {
        for (uint32_t g = 0; g < group_amount; ++g) {
            const size_t group_index = (j * group_amount + g) << 3;

            for (uint32_t word = 0; word < 8; ++word) {
                uint32_t w = mask[group_index + word];

                while (w != 0) {
                    const auto index = countr_zero(w);

                    indices[j].push_back((g << 8) + (word << 5) + index);

                    w &= ~(1U << index);
                }
            }
        }
    }

    clReleaseMemObject(mask_buffer);
    clReleaseMemObject(text_buffer);
    clReleaseMemObject(queries_buffer);
    clReleaseMemObject(offsets_buffer);
    clReleaseMemObject(lengths_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return indices;
}