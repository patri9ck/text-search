#include "candidate_opencl_v1_text_search.h"

#include <CL/opencl.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#ifdef BENCHMARK
Timer candidate_opencl_v1_timer = Timer(std::string("candidate_opencl_v1"));
#endif

// Helper to check for OpenCL errors
static void checkErr(cl_int err, const char *name) {
    if (err != CL_SUCCESS) {
        std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

static const char *kernel_source =
    "__kernel void multi_search("
    "   __global const char* book,"
    "   int book_len,"
    "   __global const char* queries,"
    "   __global const int* offsets,"
    "   __global const int* lengths,"
    "   __global int* results,"
    "   __global int* counts,"
    "   int max_matches_per_query) \n" // \n am Ende
    "{\n"
    "   int char_idx = get_global_id(0);\n"
    "   int query_idx = get_global_id(1);\n"
    "   \n"
    "   /* Boundary check using C-style comment */\n"
    "   if (char_idx > (book_len - lengths[query_idx])) return;\n"
    "   \n"
    "   int start = offsets[query_idx];\n"
    "   int len = lengths[query_idx];\n"
    "   bool match = true;\n"
    "   \n"
    "   for (int i = 0; i < len; i++) {\n"
    "       if (book[char_idx + i] != queries[start + i]) {\n"
    "           match = false; break;\n"
    "       }\n"
    "   }\n"
    "   \n"
    "   if (match) {\n"
    "       int pos = atomic_inc(&counts[query_idx]);\n"
    "       \n"
    "       if (pos < max_matches_per_query) {\n"
    "           int write_pos = (query_idx * max_matches_per_query) + pos;\n"
    "           results[write_pos] = char_idx;\n"
    "       }\n"
    "   }\n"
    "}\n";

std::vector<std::vector<size_t>>
find_candidate_opencl_v1(const std::string &text,
                         const std::vector<std::string> &queries) {
    cl_int err;
    int num_queries = (int)queries.size();
    int max_matches = 5000000; // Maximum results tracked per word
    int text_len = (int)text.size();

    cl_platform_id platform;
    checkErr(clGetPlatformIDs(1, &platform, NULL), "GetPlatform");
    cl_device_id device;
    checkErr(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL),
             "GetDevice");

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    checkErr(err, "CreateContext");
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    checkErr(err, "CreateQueue");

    cl_program program =
        clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL,
                              &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size,
                              log.data(), NULL);
        std::cerr << "Compile Error:\n" << log.data() << std::endl;
        exit(1);
    }
    cl_kernel kernel = clCreateKernel(program, "multi_search", &err);

    std::string packed_queries = "";
    std::vector<int> h_offsets, h_lengths;
    for (const auto &q : queries) {
        h_offsets.push_back((int)packed_queries.size());
        h_lengths.push_back((int)q.size());
        packed_queries += q;
    }

    cl_mem book_buf =
        clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       text_len, (void *)text.data(), &err);
    cl_mem queries_buf = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, packed_queries.size(),
        (void *)packed_queries.data(), &err);
    cl_mem offsets_buf = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        h_offsets.size() * sizeof(int), (void *)h_offsets.data(), &err);
    cl_mem lengths_buf = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        h_lengths.size() * sizeof(int), (void *)h_lengths.data(), &err);

    cl_mem results_buf =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                       num_queries * max_matches * sizeof(int), NULL, &err);
    std::vector<int> zero_counts(num_queries, 0);
    cl_mem counts_buf =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       num_queries * sizeof(int), zero_counts.data(), &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &book_buf);
    clSetKernelArg(kernel, 1, sizeof(int), &text_len);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &queries_buf);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &offsets_buf);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &lengths_buf);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), &results_buf);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), &counts_buf);
    clSetKernelArg(kernel, 7, sizeof(int), &max_matches);

    // Execute (2D Grid: Text Positions x Number of Queries)
    size_t global_size[2] = {(size_t)text_len, (size_t)num_queries};
    checkErr(clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL,
                                    0, NULL, NULL),
             "Execute");
    clFinish(queue);

    std::vector<int> final_counts(num_queries);
    std::vector<int> all_results(num_queries * max_matches);
    clEnqueueReadBuffer(queue, counts_buf, CL_TRUE, 0,
                        num_queries * sizeof(int), final_counts.data(), 0, NULL,
                        NULL);
    clEnqueueReadBuffer(queue, results_buf, CL_TRUE, 0,
                        all_results.size() * sizeof(int), all_results.data(), 0,
                        NULL, NULL);

    std::vector<std::vector<size_t>> final_indices(num_queries);
    for (int i = 0; i < num_queries; i++) {
        //std::cout << "Final counts: " << final_counts[i] << std::endl;
        int count = std::min(final_counts[i], max_matches);
        for (int j = 0; j < count; j++) {
            final_indices[i].push_back(all_results[i * max_matches + j]);
        }
    }

    // Cleanup
    clReleaseMemObject(book_buf);
    clReleaseMemObject(queries_buf);
    clReleaseMemObject(offsets_buf);
    clReleaseMemObject(lengths_buf);
    clReleaseMemObject(results_buf);
    clReleaseMemObject(counts_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return final_indices;
}